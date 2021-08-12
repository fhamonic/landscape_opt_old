#include <filesystem>
#include <fstream>
#include <iostream>

#include "lemon/connectivity.h"
#include "lemon/dijkstra.h"

#include "lemon/graph_to_eps.h"

#include "parsers/std_mutable_landscape_parser.hpp"
#include "parsers/std_restoration_plan_parser.hpp"

#include "indices/eca.hpp"
#include "landscape/decored_landscape.hpp"

#include "utils/random_chooser.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

#include "solvers/bogo.hpp"
#include "solvers/glutton_eca_dec.hpp"
#include "solvers/glutton_eca_inc.hpp"
#include "solvers/naive_eca_dec.hpp"
#include "solvers/naive_eca_inc.hpp"
#include "solvers/pl_eca_2.hpp"
#include "solvers/pl_eca_3.hpp"
#include "solvers/randomized_rounding.hpp"

#include "helper.hpp"
#include "instances_helper.hpp"

static std::vector<std::unique_ptr<concepts::Solver>> construct_solvers() {
    std::vector<std::unique_ptr<concepts::Solver>> solvers;
    int log_pl = 3;

    auto naive_eca_inc = std::make_unique<Solvers::Naive_ECA_Inc>();
    naive_eca_inc->setLogLevel(0).setParallel(true);
    solvers.emplace_back(std::move(naive_eca_inc));

    auto naive_eca_dec = std::make_unique<Solvers::Naive_ECA_Dec>();
    naive_eca_dec->setLogLevel(0).setParallel(true);
    solvers.emplace_back(std::move(naive_eca_dec));

    auto glutton_eca_inc = std::make_unique<Solvers::Glutton_ECA_Inc>();
    glutton_eca_inc->setLogLevel(0).setParallel(true);
    solvers.emplace_back(std::move(glutton_eca_inc));

    auto glutton_eca_dec = std::make_unique<Solvers::Glutton_ECA_Dec>();
    glutton_eca_dec->setLogLevel(0).setParallel(true);
    solvers.emplace_back(std::move(glutton_eca_dec));

    auto pl_eca_2 = std::make_unique<Solvers::PL_ECA_2>();
    pl_eca_2->setLogLevel(log_pl);
    solvers.emplace_back(std::move(pl_eca_2));

    auto pl_eca_3 = std::make_unique<Solvers::PL_ECA_3>();
    pl_eca_3->setLogLevel(log_pl).setTimeout(3600);
    solvers.emplace_back(std::move(pl_eca_3));

    auto randomized_rounding =
        std::make_unique<Solvers::Randomized_Rounding_ECA>();
    randomized_rounding->setLogLevel(0).setNbDraws(1000).setParallel(true);
    solvers.emplace_back(std::move(randomized_rounding));

    return solvers;
}

int main() {
    std::vector<std::unique_ptr<concepts::Solver>> solvers =
        construct_solvers();

    std::ofstream data_log("output/data.log");
    data_log << std::fixed << std::setprecision(6);
    data_log << "nb_friches "
             << "solver "
             << "time "
             << "nb_vars "
             << "nb_constraints " << std::endl;

    std::vector<double> nb_friches_values;
    for(double i = 20; i <= 170; i += 30) nb_friches_values.push_back(i);
    std::vector<double> budget_percent_values;
    for(double i = 10; i <= 90; i += 10) budget_percent_values.push_back(i);

    for(double nb_friches : nb_friches_values) {
        Instance instance = make_instance_marseillec(1, 0.04, 900, nb_friches);

        const MutableLandscape & landscape = instance.landscape;
        const RestorationPlan<MutableLandscape> & plan = instance.plan;

        Helper::assert_well_formed(landscape, plan);

        for(const std::unique_ptr<concepts::Solver> & solver : solvers) {
            const int n = budget_percent_values.size();
            double sum_time = 0;
            int nb_vars, nb_constraints;

            for(double budget_percent : budget_percent_values) {
                const double budget = (budget_percent * plan.totalCost()) / 100;
                Solution solution = solver->solve(landscape, plan, budget);

                sum_time += solution.getComputeTimeMs();
                nb_vars = solution.nb_vars;
                nb_constraints = solution.nb_constraints;
            }

            data_log << nb_friches << " " << solver->toString() << " "
                     << sum_time / n << " " << nb_vars << " " << nb_constraints
                     << " " << std::endl;
        }
    }

    return EXIT_SUCCESS;
}