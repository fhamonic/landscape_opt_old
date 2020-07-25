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
#include "solvers/naive_eca_dec.hpp"
#include "solvers/glutton_eca_dec.hpp"
#include "solvers/pl_eca_2.hpp"
#include "solvers/pl_eca_3.hpp"
#include "solvers/randomized_rounding.hpp"

static void populate(std::list<concepts::Solver*> & solvers) {
    Solvers::Bogo * bogo = new Solvers::Bogo();
    Solvers::Naive_ECA_Dec * naive_eca_dec = new Solvers::Naive_ECA_Dec();
    (*naive_eca_dec).setLogLevel(0).setParallel(true);
    Solvers::Glutton_ECA_Dec * glutton_eca_dec = new Solvers::Glutton_ECA_Dec();
    (*glutton_eca_dec).setLogLevel(0).setParallel(true);
    Solvers::PL_ECA_3 * pl_eca_3 = new Solvers::PL_ECA_3();
    (*pl_eca_3).setLogLevel(2).setNbThreads(10).setTimeout(36000000);
    Solvers::Randomized_Rounding_ECA * randomized_rounding_1000 = new Solvers::Randomized_Rounding_ECA();
    randomized_rounding_1000->setLogLevel(0).setNbDraws(1000);
    Solvers::Randomized_Rounding_ECA * randomized_rounding_10000 = new Solvers::Randomized_Rounding_ECA();
    randomized_rounding_10000->setLogLevel(0).setNbDraws(10000);

    solvers.push_back(bogo);
    solvers.push_back(naive_eca_dec);
    solvers.push_back(glutton_eca_dec);
    solvers.push_back(pl_eca_3);
    solvers.push_back(randomized_rounding_1000);
    solvers.push_back(randomized_rounding_10000);
}

static void clean(std::list<concepts::Solver*> & solvers) {
    for(concepts::Solver * solver : solvers)
        delete solver;
}

int main (int argc, const char *argv[]) {
    if(argc < 4) {
        std::cerr << "input requiered : <landscape_file> <problem_file> <alpha>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];
    std::filesystem::path problem_path = argv[2];
    const double alpha = std::atof(argv[3]);

    std::list<concepts::Solver*> solvers;
    populate(solvers);
    
    Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);

    StdRestorationPlanParser parser(*landscape);
    RestorationPlan * plan = parser.parse(problem_path);


    std::ofstream data_log("output/data.log");
    
    for(concepts::Solver * solver : solvers)
        std::cout << " " << solver->toString() << std::endl;
    std::cout << std::endl;

    double max_budget = 0.0;
    for(RestorationPlan::Option * option : plan->options())
        max_budget += option->getCost();
    
    for(int percent_budget=5; percent_budget<=95; percent_budget+=5) {
        const double B = percent_budget * max_budget / 100;
        data_log << percent_budget; data_log.flush();
        for(concepts::Solver * solver : solvers) { std::cout.flush();
            Solution * solution = solver->solve(*landscape, *plan, alpha, B);
            assert(solution != nullptr);

            data_log << " " << solution->getComputeTimeMs() << " " << ECA::get().eval(*landscape, alpha, *solution); data_log.flush();

            delete solution;
        }
        data_log << std::endl;
    }

    delete plan;
    delete landscape;

    clean(solvers);

    return EXIT_SUCCESS;
}

/*

echo "set terminal pdf;
set output 'output/time.pdf';
set style line 1 linecolor rgb '#ff0033' linetype 1 linewidth 1;
set style line 2 linecolor rgb '#0080b3' linetype 1 linewidth 1;
set key top left;
set title 'percentage of threatened arcs vs running time on a 200 patchs graph'
set ylabel 'time in ms' textcolor black;
set xlabel 'percentage of threatened arcs' textcolor black;
set xrange [5:20];
plot 'output/contraction_data/time copy.csv' using 2:3 ti 'pl\_eca\_2' with lines linestyle 1, 'output/contraction_data/time copy.csv' using 2:4 ti 'pl\_eca\_3' with lines linestyle 2" | gnuplot
*/


        