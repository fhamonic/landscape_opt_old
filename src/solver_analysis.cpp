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

#include "utils/random_chooser.hpp"

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
#include "instances_helper.hpp"

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
    Solvers::PL_ECA_2 * pl_eca_2 = new Solvers::PL_ECA_2();
    (*pl_eca_2).setLogLevel(log_pl).setNbThreads(10).setTimeout(36000);
    Solvers::PL_ECA_3 * pl_eca_3 = new Solvers::PL_ECA_3();
    (*pl_eca_3).setLogLevel(log_pl).setNbThreads(10).setTimeout(36000);
    Solvers::Randomized_Rounding_ECA * randomized_rounding_1000 = new Solvers::Randomized_Rounding_ECA();
    randomized_rounding_1000->setLogLevel(0).setNbDraws(1000);

    solvers.push_back(bogo);
    solvers.push_back(naive_eca_inc);
    solvers.push_back(naive_eca_dec);
    solvers.push_back(glutton_eca_inc);
    solvers.push_back(glutton_eca_dec);
    solvers.push_back(pl_eca_2);
    solvers.push_back(pl_eca_3);
    solvers.push_back(randomized_rounding_1000);
}
static void clean(std::list<concepts::Solver*> & solvers) {
    for(concepts::Solver * solver : solvers)
        delete solver;
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
                        Instance * instance = make_instance_quebec(pow, thresold, median, length_gain, area_gain);
                        // Instance * instance = make_instance_marseille(pow, thresold, median, length_gain, area_gain);
                        
                        const Landscape & landscape = instance->landscape;
                        const RestorationPlan & plan = instance->plan;
                        
                        // StdLandscapeParser::get().write(landscape, "output", "analysis_landscape", true);
                        // StdRestorationPlanParser(landscape).write(plan, "output", "analysis_plan", true);

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