#include <iostream>
#include <filesystem>
#include <fstream>

#include "lemon/dijkstra.h"
#include "lemon/connectivity.h"

#include "lemon/graph_to_eps.h"

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

#include "indices/eca.hpp"
#include "landscape/decored_landscape.hpp"

#include "random_chooser.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

#include "solvers/bogo.hpp"
#include "solvers/naive_eca_inc.hpp"
#include "solvers/naive_eca_dec.hpp"
#include "solvers/glutton_eca_inc.hpp"
#include "solvers/glutton_eca_dec.hpp"
#include "solvers/pl_eca_2.hpp"
#include "solvers/pl_eca_3.hpp"
#include "solvers/randomized_rounding.hpp"

#include "helper.hpp"

static void populate(std::list<concepts::Solver*> & solvers) {
    int log_pl = 2;

    Solvers::Bogo * bogo = new Solvers::Bogo(); (void)bogo;
    Solvers::Naive_ECA_Inc * naive_eca_inc = new Solvers::Naive_ECA_Inc();
    (*naive_eca_inc).setLogLevel(0).setParallel(true);
    Solvers::Naive_ECA_Dec * naive_eca_dec = new Solvers::Naive_ECA_Dec();
    (*naive_eca_dec).setLogLevel(0).setParallel(true);
    Solvers::Glutton_ECA_Inc * glutton_eca_inc = new Solvers::Glutton_ECA_Inc();
    (*glutton_eca_inc).setLogLevel(0).setParallel(true);
    Solvers::Glutton_ECA_Dec * glutton_eca_dec = new Solvers::Glutton_ECA_Dec();
    (*glutton_eca_dec).setLogLevel(0).setParallel(true);
    // Solvers::PL_ECA_2 * pl_eca_2 = new Solvers::PL_ECA_2();
    // (*pl_eca_2).setLogLevel(log_pl).setNbThreads(10).setTimeout(36000);
    Solvers::PL_ECA_3 * pl_eca_3 = new Solvers::PL_ECA_3();
    (*pl_eca_3).setLogLevel(log_pl).setNbThreads(10).setTimeout(36000);
    Solvers::Randomized_Rounding_ECA * randomized_rounding_1000 = new Solvers::Randomized_Rounding_ECA();
    randomized_rounding_1000->setLogLevel(0).setNbDraws(1000);

    solvers.push_back(bogo);
    solvers.push_back(naive_eca_inc);
    solvers.push_back(naive_eca_dec);
    solvers.push_back(glutton_eca_inc);
    solvers.push_back(glutton_eca_dec);
    // solvers.push_back(pl_eca_2);
    solvers.push_back(pl_eca_3);
    solvers.push_back(randomized_rounding_1000);
}
static void clean(std::list<concepts::Solver*> & solvers) {
    for(concepts::Solver * solver : solvers)
        delete solver;
}


class Instance {
    public:
        Landscape landscape;
        const Graph_t & graph;
        RestorationPlan plan;

        Instance() : graph(landscape.getNetwork()), plan(landscape) {}
};

Instance * make_instance_marseille(double pow, double thresold, double median, bool length_gain, bool quality_gain) {
    Instance * instance = new Instance;
    
    Landscape & landscape = instance->landscape;
    const Graph_t & graph = instance->graph;
    RestorationPlan & plan = instance->plan;

    auto d = [&landscape] (Graph_t::Node u, Graph_t::Node v) { return std::sqrt((landscape.getCoords(u) - landscape.getCoords(v)).normSquare()); };
    auto p = [median, pow] (const double d) { return std::exp(std::pow(d,pow)/std::pow(median, pow)*std::log(0.5)); };
    
    RandomChooser<Point> friches_chooser(9876);
    std::vector<Graph_t::Node> friches_list;

    io::CSVReader<3> patches("data/Marseille/patchs_marseille.patches");
    patches.read_header(io::ignore_extra_column, "category","x","y");
    std::string category;
    double x, y;
    while(patches.read_row(category, x, y)) {
        if(category.compare("\"massif\"") == 0) { landscape.addNode(20, Point(x,y)); continue; }
        if(category.compare("\"Parc\"") == 0) { landscape.addNode(5, Point(x,y)); continue; }
        if(category.compare("\"parc\"") == 0) { landscape.addNode(1, Point(x,y)); continue; }
        if(category.compare("\"friche\"") == 0) { friches_chooser.add(Point(x,y), 1); continue; }
        assert(false);
    }
    for(int i=0; i<100; i++) {
        if(!friches_chooser.canPick()) break;
        Graph_t::Node u = landscape.addNode(0, friches_chooser.pick());
        friches_list.push_back(u);
    }

    for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) {
        for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
            if(u == v) continue;
            double dist = d(u,v);
            double probability = p(dist);
            if(probability < thresold) continue;
            landscape.addArc(u, v, probability);
        }
    }

    int cpt = 0;
    for(Graph_t::Node v1 : friches_list) {
        RestorationPlan::Option * option = plan.addOption();
        option->setCost(1);
        option->setId(cpt);
        cpt++;
        if(length_gain) {
            Graph_t::Node v2 = landscape.addNode(0, landscape.getCoords(v1) + Point(0.0001, 0.0001));
            std::vector<Graph_t::Arc> to_move;
            for(Graph_t::InArcIt a(graph, v1); a != lemon::INVALID; ++a)
                to_move.push_back(a);
            for(Graph_t::Arc a : to_move)
                landscape.changeTarget(a, v2);

            Graph_t::Arc v1v2 = landscape.addArc(v2, v1, std::numeric_limits<double>::epsilon());
            option->addLink(v1v2, 1.0);
        }
        if(quality_gain)
            option->addPatch(v1, quality_gain);
    }

    return instance;
}


Instance * make_instance_quebec(double pow, double thresold, double median, bool length_gain, bool quality_gain) {
    Instance * instance = new Instance;
    
    Landscape & landscape = instance->landscape;
    const Graph_t & graph = instance->graph;
    RestorationPlan & plan = instance->plan;

    auto p = [median, pow] (const double d) { return std::exp(std::pow(d,pow)/std::pow(median, pow)*std::log(0.5)); };
    
    std::vector<Graph_t::Node> node_correspondance;
    std::vector<Graph_t::Node> total_threaten;
    int cpt = 0;

    io::CSVReader<5> patches("data/quebec_leam_v3/raw/sommets_leam_v3.txt");
    patches.read_header(io::ignore_extra_column, "count","area","xcoord","ycoord","count2050");
    int count;
    double area, xcoord, ycoord, count2050;
    while(patches.read_row(count, area, xcoord, ycoord, count2050)) {
        node_correspondance.push_back(lemon::INVALID);
        if(xcoord < 240548.456514) continue;
        if(xcoord >= 263015.4829015) continue;
        if(ycoord < 4986893.0) continue;
        if(ycoord >= 5003751.29081625) continue;
        
        Graph_t::Node u = landscape.addNode(count2050, Point(xcoord,ycoord));
        assert(static_cast<int>(node_correspondance.size()) == count);
        node_correspondance[count-1] = u;

        if(area == count2050)
            continue;
        if(area > 0 && count2050 == 0) {
            total_threaten.push_back(u);
            continue;
        }
        RestorationPlan::Option * option = plan.addOption();
        option->setCost(1);
        option->setId(cpt);
        cpt++;
        option->addPatch(u, area-count2050);
    }

    io::CSVReader<3> links("data/quebec_leam_v3/raw/aretes_leam_v3.txt");
    links.read_header(io::ignore_extra_column, "from","to","Dist");
    int from, to;
    double Dist;
    while(links.read_row(from, to, Dist)) {
        Graph_t::Node u = node_correspondance[from-1];
        Graph_t::Node v = node_correspondance[to-1];
        if(u == lemon::INVALID || v == lemon::INVALID) continue;
        double probability = p(Dist);
        if(probability < thresold) continue;
        landscape.addArc(u, v, probability);
    }

    for(Graph_t::Node v1 : total_threaten) {
        RestorationPlan::Option * option = plan.addOption();
        option->setCost(1);
        option->setId(cpt);
        cpt++;
        if(length_gain) {
            Graph_t::Node v2 = landscape.addNode(0, landscape.getCoords(v1) + Point(0.0001, 0.0001));
            std::vector<Graph_t::Arc> to_move;
            for(Graph_t::InArcIt a(graph, v1); a != lemon::INVALID; ++a)
                to_move.push_back(a);
            for(Graph_t::Arc a : to_move)
                landscape.changeTarget(a, v2);

            Graph_t::Arc v2v1 = landscape.addArc(v2, v1, std::numeric_limits<double>::epsilon());
            option->addLink(v2v1, 1.0);
        }
        if(quality_gain)
            option->addPatch(v1, quality_gain);
    }

    return instance;
}




int main() {
    std::list<concepts::Solver*> solvers;
    populate(solvers);
    
    std::ofstream data_log("output/data.log");
    data_log << std::fixed << std::setprecision(6);
    data_log << "pow "
            << "thresold "
            << "length_gain "
            << "area_gain "
            << "median "
            << "budget "
            << "solver "
            << "time "
            << "cost "
            << "objective "
            << "total_eca "
            << "eval_pl_eca_2 "
            << "eval_pl_eca_3 "
            << std::endl;

    std::vector<double> pow_values{1, 2};
    std::vector<double> thresold_values{0.01};
    std::vector<double> median_values{700, 1400, 2800}; 
    std::vector<bool> length_gain_values{true}; 
    std::vector<bool> area_gain_values{/*false, */true};
    std::vector<double> budget_values;
    for(int i=0; i<=20; i+=2) budget_values.push_back(i);

    const ECA & eca = ECA::get();

    for(double pow : pow_values) {
        for(double thresold : thresold_values) {
            for(double median : median_values) {
                for(bool length_gain : length_gain_values) {
                    for(bool area_gain : area_gain_values) {
                        if(length_gain == 0 && area_gain == 0) continue;
                        
                        // Instance * instance = make_instance_quebec(pow, thresold, median, length_gain, area_gain);
                        Instance * instance = make_instance_marseille(pow, thresold, median, length_gain, area_gain);
                        
                        const Landscape & landscape = instance->landscape;
                        const RestorationPlan & plan = instance->plan;
                        
                        StdLandscapeParser::get().write(landscape, "output", "analysis_landscape", true);
                        StdRestorationPlanParser(landscape).write(plan, "output", "analysis_plan", true);

                        Helper::assert_well_formed(landscape, plan);

                        for(double budget : budget_values) {
                            for(concepts::Solver * solver : solvers) {
                                Solution * solution = solver->solve(landscape, plan, budget);

                                double cost = 0.0;
                                for(auto option_pair : solution->getOptionCoefs())
                                    cost += option_pair.first->getCost() * option_pair.second;
                                const double total_eca = std::pow(eca.eval_solution(landscape, *solution), 2);

                                Solvers::PL_ECA_2 pl_eca_2;
                                double eval_pl_eca_2 = pl_eca_2.eval(landscape, plan, budget, *solution);
                                Solvers::PL_ECA_3 pl_eca_3;
                                double eval_pl_eca_3 = pl_eca_3.eval(landscape, plan, budget, *solution);

                                data_log << pow << " " 
                                        << thresold << " " 
                                        << length_gain << " " 
                                        << area_gain << " " 
                                        << median << " "
                                        << budget << " "
                                        << solver->toString() << " "
                                        << solution->getComputeTimeMs() << " "
                                        << cost << " "
                                        << solution->obj << " "
                                        << total_eca << " "
                                        << eval_pl_eca_2 << " "
                                        << eval_pl_eca_3
                                        << std::endl;

                                delete solution;
                            }



                            // delete instance;
                            // clean(solvers);
                            // return 1;
                        }   
                        delete instance;
                    }             
                }
            }
        }
    }

    clean(solvers);

    return EXIT_SUCCESS;
}        