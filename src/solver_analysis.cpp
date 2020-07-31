#include <iostream>
#include <filesystem>
#include <fstream>

#include "lemon/dijkstra.h"
#include "lemon/connectivity.h"

#include "lemon/graph_to_eps.h"

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

#include "random_chooser.hpp"

#include "helper.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

#include "solvers/bogo.hpp"
#include "solvers/naive_eca_inc.hpp"
#include "solvers/naive_eca_dec.hpp"
#include "solvers/glutton_eca_inc.hpp"
#include "solvers/glutton_eca_dec.hpp"
#include "solvers/pl_eca_2.hpp"
#include "solvers/pl_eca_3.hpp"
#include "solvers/randomized_rounding.hpp"

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
    Solvers::PL_ECA_3 * pl_eca_3 = new Solvers::PL_ECA_3();
    (*pl_eca_3).setLogLevel(1).setNbThreads(10).setTimeout(36000);
    Solvers::Randomized_Rounding_ECA * randomized_rounding_1000 = new Solvers::Randomized_Rounding_ECA();
    randomized_rounding_1000->setLogLevel(0).setNbDraws(1000);
    Solvers::Randomized_Rounding_ECA * randomized_rounding_10000 = new Solvers::Randomized_Rounding_ECA();
    randomized_rounding_10000->setLogLevel(1).setNbDraws(10000).setParallel(true);

    // solvers.push_back(bogo);
    // solvers.push_back(naive_eca_inc);
    // solvers.push_back(naive_eca_dec);
    solvers.push_back(glutton_eca_inc);
    solvers.push_back(glutton_eca_dec);
    solvers.push_back(pl_eca_3);
    // solvers.push_back(randomized_rounding_1000);
    // solvers.push_back(randomized_rounding_10000);
}

static void clean(std::list<concepts::Solver*> & solvers) {
    for(concepts::Solver * solver : solvers)
        delete solver;
}

RestorationPlan * make_instance(Landscape & landscape, Graph_t::NodeMap<bool> & friches_filter, double length_gain, double quality_gain, const double alpha) {
    const Graph_t & graph = landscape.getNetwork();
    RestorationPlan * plan = new RestorationPlan(landscape);
    int cpt=0;
    std::vector<Graph_t::Node> friches;
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        if(!friches_filter[v]) continue;
        friches.push_back(v);
    }

    for(Graph_t::Node v1 : friches) {
        RestorationPlan::Option * option = plan->addOption();
        option->setCost(1);
        option->setId(cpt);
        cpt++;
        if(length_gain > 0) {
            //*
            Graph_t::Node v2 = landscape.addNode(0, landscape.getCoords(v1));
            // const double scale_arcs_probability = std::exp(length_gain/2/alpha);
            // for(Graph_t::InArcIt a(graph, v1); a != lemon::INVALID; ++a)
            //     landscape.getProbabilityRef(a) = std::min(1.0, scale_arcs_probability * landscape.getProbability(a));
            std::vector<Graph_t::Arc> to_move;
            for(Graph_t::OutArcIt a(graph, v1); a != lemon::INVALID; ++a) {
                //landscape.getProbabilityRef(a) *= std::min(1.0, scale_arcs_probability * landscape.getProbability(a));
                to_move.push_back(a);
            }
            for(Graph_t::Arc a : to_move)
                landscape.changeSource(a, v2);
            Graph_t::Arc v1v2 = landscape.addArc(v1, v2, std::numeric_limits<double>::epsilon());

            option->addLink(v1v2, 1.0);
            /*/
            const double scale_arcs_probability = std::exp(length_gain/2/alpha);
            for(Graph_t::InArcIt a(graph, v1); a != lemon::INVALID; ++a)
                option->addLink(a, std::min(1.0, scale_arcs_probability * landscape.getProbability(a)));
            for(Graph_t::OutArcIt a(graph, v1); a != lemon::INVALID; ++a)
                option->addLink(a, std::min(1.0, scale_arcs_probability * landscape.getProbability(a)));
                          

            //*/
        }
        if(quality_gain > 0)
            option->addPatch(v1, quality_gain);
    }
    return plan;
}

void seuiller(Landscape & landscape, const double thresold) {
    const Graph_t & graph = landscape.getNetwork();
    std::vector<Graph_t::ArcIt> arcs_to_delete;
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a)
        if(landscape.getProbability(a) < thresold)
            arcs_to_delete.push_back(a);
    for(Graph_t::ArcIt a : arcs_to_delete)
        landscape.removeArc(a);
}

void count(Landscape & landscape, Graph_t::NodeMap<bool> & filter) {
    const Graph_t & graph = landscape.getNetwork();
    int cpt = 0;
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        if(!filter[v])
            continue;
        cpt++;
    }
    std::cout << cpt << std::endl;
}

int main (int argc, const char *argv[]) {
    if(argc < 2) {
        std::cerr << "input requiered : <landscape_file>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];

    std::list<concepts::Solver*> solvers;
    populate(solvers);
    
    std::ofstream data_log("output/data.log");
    data_log << std::fixed << std::setprecision(6);
    data_log << "thresold " <<
            "length_gain " <<
            "area_gain " <<
            "alpha " <<
            "budget " <<
            "solver " <<
            "time " <<
            "cost " <<
            "total_eca " <<
            "massifs_eca " <<
            "parcs_eca " <<
            "massifs_parcs_eca" << std::endl;

    std::vector<double> thresold_values{0.01};
    std::vector<double> median_values{/*350,*/ 1400, 2800}; 
    std::vector<double> length_gain_values{30, 100, 200}; 
    std::vector<double> area_gain_values{0, 0.5};
    std::vector<double> budget_values;
    for(int i=5; i<=100; i+=5) budget_values.push_back(i);

    ECA & eca = ECA::get();

    auto p = [] (const double d, const double median, const double pow) { return std::exp(std::pow(d,pow)/std::pow(median, pow)*std::log(0.5)); };

    for(double thresold : thresold_values) {
        for(double median : median_values) {
            for(double length_gain : length_gain_values) {
                for(double area_gain : area_gain_values) {
                    Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);
                    const Graph_t & graph = landscape->getNetwork();

                    Graph_t::NodeMap<bool> massifs(graph, false);
                    Graph_t::NodeMap<bool> parcs(graph, false);
                    Graph_t::NodeMap<bool> friches(graph, false);
                    Graph_t::NodeMap<bool> massifs_or_parcs(graph, false);

                    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
                        const int quality = landscape->getQuality(v);
                        switch(quality) {
                            case 3: massifs[v] = true; landscape->setQuality(v, 10);
                                break;
                            case 25: parcs[v] = true; landscape->setQuality(v, 5);
                                break;
                            case 21: parcs[v] = true; landscape->setQuality(v, 1);
                                break;
                            case 1: friches[v] = true; landscape->setQuality(v, 0);
                                break;
                            default: assert(false);
                        }
                        massifs_or_parcs[v] = parcs[v] || massifs[v];
                    }

                    count(*landscape, massifs);
                    count(*landscape, parcs);
                    count(*landscape, friches);
                    count(*landscape, massifs_or_parcs);


                    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
                        landscape->setProbability(a, p( landscape->getProbability(a) , median , 2 ));
                    }

                    seuiller(*landscape, thresold);

                    // StdLandscapeParser::get().write(*landscape, "output", "landscape");

            
                    RestorationPlan * plan = make_instance(*landscape, friches, length_gain, area_gain, median);


                    StdLandscapeParser::get().write(*landscape, "output", "analysis_landscape", true);
                    StdRestorationPlanParser(*landscape).write(*plan, "output", "analysis_plan", true);


                    Helper::assert_well_formed(*landscape, *plan);



                    for(double budget : budget_values) {
                        for(concepts::Solver * solver : solvers) {

                            std::cout << "before solve : " << pow(eca.eval(*landscape), 2) << std::endl;

                            Solution * solution = solver->solve(*landscape, *plan, budget);

                            double cost = 0.0;
                            DecoredLandscape decored_landscape(*landscape);
                            for(auto option_pair : solution->getOptionCoefs()) {
                                cost += option_pair.first->getCost() * option_pair.second;
                                decored_landscape.apply(option_pair.first, option_pair.second);   
                            }

                            const double total_eca = pow(eca.eval(decored_landscape), 2);
                            const double massifs_eca = pow(eca.eval(decored_landscape, massifs), 2);
                            const double parcs_eca = pow(eca.eval(decored_landscape, parcs), 2);

                            const double massifs_parcs_eca = pow(eca.eval(decored_landscape, massifs_or_parcs), 2) - massifs_eca - parcs_eca;

                            data_log << thresold << " " 
                                    << length_gain << " " 
                                    << area_gain << " " 
                                    << median << " "
                                    // << alpha << " "
                                    << budget << " "
                                    << solver->toString() << " "
                                    << solution->getComputeTimeMs() << " "
                                    << cost << " "
                                    << total_eca << " "
                                    << massifs_eca << " "
                                    << parcs_eca << " "
                                    << massifs_parcs_eca << std::endl;

                            delete solution;
                        }
                    }   
                    delete plan;
                    delete landscape;
                }             
            }
        }
    }

    clean(solvers);

    return EXIT_SUCCESS;
}        