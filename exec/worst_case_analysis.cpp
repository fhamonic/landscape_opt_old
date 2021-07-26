#include <iostream>
#include <filesystem>
#include <fstream>

#include "lemon/dijkstra.h"
#include "lemon/connectivity.h"

#include "lemon/graph_to_eps.h"

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_mutable_landscape_parser.hpp"

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
#include "print_helper.hpp"
#include "instances_helper.hpp"

static void populate(std::list<concepts::Solver*> & solvers) {
    int log_pl = 1;

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

    solvers.push_back(naive_eca_inc);
    solvers.push_back(naive_eca_dec);
    solvers.push_back(glutton_eca_inc);
    solvers.push_back(glutton_eca_dec);
    // solvers.push_back(pl_eca_2);
    solvers.push_back(pl_eca_3);
    solvers.push_back(randomized_rounding);
}
static void clean(std::list<concepts::Solver*> & solvers) {
    for(concepts::Solver * solver : solvers)
        delete solver;
}


int main(int argc, const char *argv[]) {
    std::list<concepts::Solver*> solvers;
    populate(solvers);
    if(argc < 3) {
        std::cerr << "input requiered : <worst case dir> <worst case name>" << std::endl;
        clean(solvers);
        return EXIT_FAILURE;
    }
    std::filesystem::path path = argv[1];
    std::string name = argv[2];
    
    std::ofstream data_log(path / "data.log");
    data_log << std::fixed << std::setprecision(6);
    data_log << "budget "
            << "solver "
            << "time "
            << "cost "
            << "total_eca "
            << std::endl;

    const ECA & eca = ECA();

    MutableLandscape * landscape = StdMutableLandscapeParser::get().parse(path / (name + ".index"));
    StdRestorationPlanParser parser(*landscape);
    RestorationPlan<MutableLandscape> * plan = parser.parse(path / (name + ".problem"));

    std::vector<double> budget_values;
    for(double i=0; i<=plan->totalCost(); i+=1) budget_values.push_back(i);

    Helper::printInstance(*landscape, *plan, path / (name + ".eps"));

    for(double budget : budget_values) {
        const int nb_bogo = 100;
        Solvers::Bogo bogo;
        double total_bogo_time = 0;
        double total_bogo_cost = 0;
        double total_bogo_eca = 0;
        for(int i=0; i<nb_bogo; ++i) {
            bogo.setSeed(i);
            Solution * solution = bogo.solve(*landscape, *plan, budget);
            total_bogo_time += solution->getComputeTimeMs();
            total_bogo_cost += solution->getCost();
            total_bogo_eca += std::pow(eca.eval_solution(*landscape, *plan, *solution), 2);
            delete solution;
        }
        data_log << budget << " "
                << bogo.toString() << " "
                << total_bogo_time / nb_bogo << " "
                << total_bogo_cost / nb_bogo << " "
                << total_bogo_eca / nb_bogo << " "
                << std::endl;

        for(concepts::Solver * solver : solvers) {
            Solution solution = solver->solve(*landscape, *plan, budget);

            const double cost = solution->getCost();
            const double total_eca = std::pow(ECA.eval(Helper::decore_landscape(*landscape, *plan, *solution)), 2);

            data_log << budget << " "
                    << solver->toString() << " "
                    << solution->getComputeTimeMs() << " "
                    << cost << " "
                    << total_eca << " "
                    << std::endl;

            delete solution;
        }

    } 
    delete plan;
    delete landscape;

    clean(solvers);

    return EXIT_SUCCESS;
}