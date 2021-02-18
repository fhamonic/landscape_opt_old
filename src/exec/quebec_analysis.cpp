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
    int log_pl = 3;

    Solvers::Naive_ECA_Inc * naive_eca_inc = new Solvers::Naive_ECA_Inc();
    (*naive_eca_inc).setLogLevel(0).setParallel(true);
    Solvers::Naive_ECA_Dec * naive_eca_dec = new Solvers::Naive_ECA_Dec();
    (*naive_eca_dec).setLogLevel(0).setParallel(true);
    Solvers::Glutton_ECA_Inc * glutton_eca_inc = new Solvers::Glutton_ECA_Inc();
    (*glutton_eca_inc).setLogLevel(0).setParallel(true);
    Solvers::Glutton_ECA_Dec * glutton_eca_dec = new Solvers::Glutton_ECA_Dec();
    (*glutton_eca_dec).setLogLevel(0).setParallel(true);
    Solvers::PL_ECA_2 * pl_eca_2 = new Solvers::PL_ECA_2();
    (*pl_eca_2).setLogLevel(log_pl);
    Solvers::PL_ECA_3 * pl_eca_3 = new Solvers::PL_ECA_3();
    (*pl_eca_3).setLogLevel(log_pl).setTimeout(3600);
    Solvers::Randomized_Rounding_ECA * randomized_rounding = new Solvers::Randomized_Rounding_ECA();
    randomized_rounding->setLogLevel(0).setNbDraws(1000).setParallel(true);

    // solvers.push_back(naive_eca_inc);
    // solvers.push_back(naive_eca_dec);
    // solvers.push_back(glutton_eca_inc);
    solvers.push_back(glutton_eca_dec);
    // solvers.push_back(pl_eca_2);
    solvers.push_back(pl_eca_3);
    // solvers.push_back(randomized_rounding);
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
            << "median "
            << "orig "
            << "budget_percent "
            << "budget "
            << "solver "
            << "time "
            << "cost "
            << "total_eca "
            << std::endl;

    std::vector<double> pow_values{1};
    std::vector<double> thresold_values{0.01};
    std::vector<double> median_values{900};
    std::vector<Point> orig_values;

    std::default_random_engine gen(9346);
    std::uniform_int_distribution<int> x_dis(240548, 387924);
    std::uniform_int_distribution<int> y_dis(4986893, 5101759);
    for(int i=0; i<100; ++i) orig_values.emplace_back(x_dis(gen), y_dis(gen));
    
    std::vector<double> budget_percent_values;
    for(double i=0; i<=20; i+=1) budget_percent_values.push_back(i);

    const ECA & eca = ECA::get();

    int min_arcs = std::numeric_limits<int>::max();
    int max_arcs = 0;

    for(double pow : pow_values) {
        for(double thresold : thresold_values) {
            for(double median : median_values) {
                for(Point orig : orig_values) {
                    Instance * instance = make_instance_quebec(pow, thresold, median, orig, Point(32360, 20000));
                    
                    const Landscape & landscape = instance->landscape;
                    const RestorationPlan<Landscape> & plan = instance->plan;

                    if(lemon::countNodes(landscape.getNetwork()) < 150) continue;
                    if(lemon::countNodes(landscape.getNetwork()) > 400) continue;

                    const int arcs = lemon::countArcs(landscape.getNetwork());
                    min_arcs = std::min(min_arcs, arcs);
                    max_arcs = std::max(max_arcs, arcs);

                    // continue;
                    
                    Helper::assert_well_formed(landscape, plan);
                    Helper::printInstance(landscape, plan, "quebec-(" + std::to_string(orig.x) + "," + std::to_string(orig.y) + ").eps");

                    for(double budget_percent : budget_percent_values) {
                        const double budget = (budget_percent * plan.totalCost())/100;

                        const int nb_bogo = 100;
                        Solvers::Bogo bogo;
                        double total_bogo_time = 0;
                        double total_bogo_cost = 0;
                        double total_bogo_eca = 0;
                        for(int i=0; i<nb_bogo; ++i) {
                            bogo.setSeed(i);
                            Solution * solution = bogo.solve(landscape, plan, budget);
                            total_bogo_time += solution->getComputeTimeMs();
                            total_bogo_cost += solution->getCost();
                            total_bogo_eca += std::pow(eca.eval_solution(landscape, plan, *solution), 2);
                            delete solution;
                        }
                        data_log << pow << " " 
                                    << thresold << " "
                                    << median << " "
                                    << orig << " "
                                    << budget_percent << " "
                                    << budget << " "
                                    << bogo.toString() << " "
                                    << total_bogo_time / nb_bogo << " "
                                    << total_bogo_cost / nb_bogo << " "
                                    << total_bogo_eca / nb_bogo << " "
                                    << std::endl;

                        for(concepts::Solver * solver : solvers) {
                            Solution * solution = solver->solve(landscape, plan, budget);

                            const double total_eca = std::pow(eca.eval_solution(landscape, plan, *solution), 2);

                            data_log << pow << " " 
                                    << thresold << " "
                                    << median << " "
                                    << orig << " "
                                    << budget_percent << " "
                                    << budget << " "
                                    << solver->toString() << " "
                                    << solution->getComputeTimeMs() << " "
                                    << solution->getCost() << " "
                                    << total_eca << " "
                                    << std::endl;

                            delete solution;
                        }

                    } 
                    delete instance;
                }
            }
        }
    }

    std::cout << "min_arcs " << min_arcs << std::endl << "max_arcs " << max_arcs << std::endl;

    clean(solvers);

    return EXIT_SUCCESS;
}