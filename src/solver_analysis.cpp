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
    Solvers::Bogo * bogo = new Solvers::Bogo(); (void)bogo;
    Solvers::Naive_ECA_Inc * naive_eca_inc = new Solvers::Naive_ECA_Inc();
    (*naive_eca_inc).setLogLevel(0).setParallel(true);
    Solvers::Naive_ECA_Dec * naive_eca_dec = new Solvers::Naive_ECA_Dec();
    (*naive_eca_dec).setLogLevel(0).setParallel(true);
    Solvers::Glutton_ECA_Inc * glutton_eca_inc = new Solvers::Glutton_ECA_Inc();
    (*glutton_eca_inc).setLogLevel(0).setParallel(true);
    Solvers::Glutton_ECA_Dec * glutton_eca_dec = new Solvers::Glutton_ECA_Dec();
    (*glutton_eca_dec).setLogLevel(0).setParallel(true);
    Solvers::PL_ECA_2 * pl_eca_2 = new Solvers::PL_ECA_2();
    (*pl_eca_2).setLogLevel(2).setNbThreads(10).setTimeout(36000);
    Solvers::PL_ECA_3 * pl_eca_3 = new Solvers::PL_ECA_3();
    (*pl_eca_3).setLogLevel(2).setNbThreads(10).setTimeout(36000);
    Solvers::Randomized_Rounding_ECA * randomized_rounding_1000 = new Solvers::Randomized_Rounding_ECA();
    randomized_rounding_1000->setLogLevel(0).setNbDraws(1000);
    Solvers::Randomized_Rounding_ECA * randomized_rounding_10000 = new Solvers::Randomized_Rounding_ECA();
    randomized_rounding_10000->setLogLevel(1).setNbDraws(10000).setParallel(true);

    solvers.push_back(bogo);
    solvers.push_back(naive_eca_inc);
    solvers.push_back(naive_eca_dec);
    solvers.push_back(glutton_eca_inc);
    solvers.push_back(glutton_eca_dec);
    // solvers.push_back(pl_eca_2);
    solvers.push_back(pl_eca_3);
    solvers.push_back(randomized_rounding_1000);
    // solvers.push_back(randomized_rounding_10000);
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
        Graph_t::NodeMap<bool> massifs;
        Graph_t::NodeMap<bool> parcs;
        Graph_t::NodeMap<bool> friches;
        Graph_t::NodeMap<bool> massifs_or_parcs;

        Instance() : graph(landscape.getNetwork()), plan(landscape), 
            massifs(graph, false), parcs(graph, false),
            friches(graph, false), massifs_or_parcs(graph, false) {}
};

Instance * make_instance(double pow, double thresold, double median, double length_gain, double quality_gain) {
    Instance * instance = new Instance;
    
    Landscape & landscape = instance->landscape;
    const Graph_t & graph = instance->graph;
    RestorationPlan & plan = instance->plan;
    Graph_t::NodeMap<bool> & massifs = instance->massifs;
    Graph_t::NodeMap<bool> & parcs = instance->parcs;
    Graph_t::NodeMap<bool> & friches = instance->friches;
    Graph_t::NodeMap<bool> & massifs_or_parcs = instance->massifs_or_parcs;

    auto d = [&landscape] (Graph_t::Node u, Graph_t::Node v) { return std::sqrt((landscape.getCoords(u) - landscape.getCoords(v)).normSquare()); };
    auto p = [median, pow] (const double d) { return std::exp(std::pow(d,pow)/std::pow(median, pow)*std::log(0.5)); };
    
    RandomChooser<Point> friches_chooser(12345);
    std::vector<Graph_t::Node> friches_list;

    io::CSVReader<3> patches("data/Marseille/patchs_marseille.patches");
    patches.read_header(io::ignore_extra_column, "category","x","y");
    std::string category;
    double x, y;
    while(patches.read_row(category, x, y)) {
        if(category.compare("\"massif\"") == 0) { massifs[landscape.addNode(20, Point(x,y))] = true; continue; }
        if(category.compare("\"Parc\"") == 0) { parcs[landscape.addNode(5, Point(x,y))] = true; continue; }
        if(category.compare("\"parc\"") == 0) { parcs[landscape.addNode(1, Point(x,y))] = true; continue; }
        if(category.compare("\"friche\"") == 0) { friches_chooser.add(Point(x,y), 1); continue; }
        assert(false);
    }
    for(int i=0; i<100; i++) {
        if(!friches_chooser.canPick()) break;
        Graph_t::Node u = landscape.addNode(0, friches_chooser.pick());
        friches_list.push_back(u);
        friches[u] = true;
    }
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v)
        massifs_or_parcs[v] = parcs[v] || massifs[v];


    for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) {
        for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
            if(u == v) continue;
            double dist = d(u,v);
            // if(dist > thresold) continue;
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
        if(length_gain > 0) {
            Graph_t::Node v2 = landscape.addNode(0, landscape.getCoords(v1) + Point(0.0001, 0.0001));
            std::vector<Graph_t::Arc> to_move;
            for(Graph_t::InArcIt a(graph, v1); a != lemon::INVALID; ++a)
                to_move.push_back(a);
            for(Graph_t::Arc a : to_move)
                landscape.changeTarget(a, v2);

            Graph_t::Arc v1v2 = landscape.addArc(v2, v1, std::numeric_limits<double>::epsilon());
            option->addLink(v1v2, 1.0);
        }
        if(quality_gain > 0)
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
            << "total_eca "
            // << "massifs_eca "
            // << "parcs_eca "
            // << "massifs_parcs_eca"
            << std::endl;

    std::vector<double> pow_values{1, 2};
    std::vector<double> thresold_values{0.01};
    std::vector<double> median_values{/*350,*/ 1400, 2800}; 
    std::vector<double> length_gain_values{0, 1}; 
    std::vector<double> area_gain_values{0, 1};
    std::vector<double> budget_values;
    for(int i=5; i<=100; i+=5) budget_values.push_back(i);

    const ECA & eca = ECA::get();

    for(double pow : pow_values) {
        for(double thresold : thresold_values) {
            for(double median : median_values) {
                for(double length_gain : length_gain_values) {
                    for(double area_gain : area_gain_values) {
                        if(length_gain == 0 && area_gain == 0) continue;
                        
                        Instance * instance = make_instance(1, thresold, median, length_gain, area_gain);
                        
                        const Landscape & landscape = instance->landscape;
                        const RestorationPlan & plan = instance->plan;
                        // const Graph_t::NodeMap<bool> & massifs = instance->massifs;
                        // const Graph_t::NodeMap<bool> & parcs = instance->parcs;
                        // const Graph_t::NodeMap<bool> & massifs_or_parcs = instance->massifs_or_parcs;
                        
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


                                // DecoredLandscape decored_landscape(landscape);
                                // for(auto option_pair : solution->getOptionCoefs()) {
                                //     decored_landscape.apply(option_pair.first, option_pair.second);   
                                // }

                                // // const double massifs_eca = pow(eca.eval_partial(decored_landscape, massifs), 2);
                                // // const double parcs_eca = pow(eca.eval(decored_landscape, parcs), 2);
                                // // const double massifs_parcs_eca = pow(eca.eval(decored_landscape, massifs_or_parcs), 2) - massifs_eca - parcs_eca;

                                data_log << pow << " " 
                                        << thresold << " " 
                                        << length_gain << " " 
                                        << area_gain << " " 
                                        << median << " "
                                        << budget << " "
                                        << solver->toString() << " "
                                        << solution->getComputeTimeMs() << " "
                                        << cost << " "
                                        << total_eca << " "
                                        // << massifs_eca << " "
                                        // << parcs_eca << " "
                                        // << massifs_parcs_eca
                                        << std::endl;

                                delete solution;
                            }
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