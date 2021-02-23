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
    int log_pl = 1;

    // Solvers::Naive_ECA_Inc * naive_eca_inc = new Solvers::Naive_ECA_Inc();
    // (*naive_eca_inc).setLogLevel(0).setParallel(true);
    // Solvers::Naive_ECA_Dec * naive_eca_dec = new Solvers::Naive_ECA_Dec();
    // (*naive_eca_dec).setLogLevel(0).setParallel(true);
    // Solvers::Glutton_ECA_Inc * glutton_eca_inc = new Solvers::Glutton_ECA_Inc();
    // (*glutton_eca_inc).setLogLevel(0).setParallel(true);
    // Solvers::Glutton_ECA_Dec * glutton_eca_dec = new Solvers::Glutton_ECA_Dec();
    // (*glutton_eca_dec).setLogLevel(0).setParallel(true);
    // Solvers::PL_ECA_2 * pl_eca_2 = new Solvers::PL_ECA_2();
    // (*pl_eca_2).setLogLevel(log_pl).setTimeout(3600);
    Solvers::PL_ECA_3 * pl_eca_3 = new Solvers::PL_ECA_3();
    (*pl_eca_3).setLogLevel(log_pl).setTimeout(3600);
    // Solvers::Randomized_Rounding_ECA * randomized_rounding_1000 = new Solvers::Randomized_Rounding_ECA();
    // randomized_rounding_1000->setLogLevel(0).setNbDraws(1000).setParallel(true);

    // solvers.push_back(naive_eca_inc);
    // solvers.push_back(naive_eca_dec);
    // solvers.push_back(glutton_eca_inc);
    // solvers.push_back(glutton_eca_dec);
    // solvers.push_back(pl_eca_2);
    solvers.push_back(pl_eca_3);
    // solvers.push_back(randomized_rounding_1000);
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
    data_log << "nb_friches "
            << "solver "
            << "time "
            << "nb_vars "
            << "nb_constraints "
            << std::endl;

    std::vector<double> nb_friches_values;
    for(double i=90; i<=300; i+=10) nb_friches_values.push_back(i);
    std::vector<double> budget_percent_values;
    for(double i=10; i<=90; i+=10) budget_percent_values.push_back(i);

    for(double nb_friches : nb_friches_values) {
        Instance * instance = make_instance_marseillec(1, 0.04, 900, nb_friches);
        
        const Landscape & landscape = instance->landscape;
        const RestorationPlan<Landscape> & plan = instance->plan;
        
        Helper::assert_well_formed(landscape, plan);

        for(concepts::Solver * solver : solvers) {
            const int n = budget_percent_values.size();
            double sum_time = 0;
            int nb_vars, nb_constraints;
            
            
            for(double budget_percent : budget_percent_values) {
                const double budget = (budget_percent * plan.totalCost())/100;
                Solution * solution = solver->solve(landscape, plan, budget);

                sum_time += solution->getComputeTimeMs();
                nb_vars = solution->nb_vars;
                nb_constraints = solution->nb_constraints;

                delete solution;
            }

            data_log << nb_friches << " "
                    << solver->toString() << " "
                    << sum_time / n << " "
                    << nb_vars << " "
                    << nb_constraints << " "
                    << std::endl;
        } 
        delete instance;
    }

    clean(solvers);

    return EXIT_SUCCESS;
}