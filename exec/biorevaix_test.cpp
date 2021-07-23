#include <iostream>
#include <filesystem>
#include <fstream>

#include "lemon/dijkstra.h"
#include "lemon/connectivity.h"

#include "lemon/graph_to_eps.h"

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

#include "indices/eca.hpp"
#include "indices/parallel_eca.hpp"

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

#include "boost/range/algorithm/count_if.hpp"

int main() {
    Instance instance = make_instance_biorevaix_level_1(3, Point(897286.5,6272835.5), 400);
    // Instance instance = make_instance_biorevaix_level_2(4);
    const Landscape & landscape = instance.landscape;
    RestorationPlan<Landscape> & plan = instance.plan;
    plan.initElementIDs();

    Helper::assert_well_formed(landscape, plan);

    std::cout << std::setprecision(8);

    std::cout << "nb nodes:" << lemon::countNodes(landscape.getNetwork()) << std::endl;
    std::cout << "nb arcs:" << lemon::countArcs(landscape.getNetwork()) << std::endl;
        
    int count = 0;
    for(Graph_t::NodeIt u(landscape.getNetwork()); u != lemon::INVALID; ++u)
        count += (landscape.getQuality(u)>0 ? 1 : 0);    
    std::cout << "nb nodes positive quality:" << count << std::endl;
    std::cout << "nb options:" << plan.getNbOptions() << std::endl;
    std::cout << "nb restorable arcs:" << plan.getNbArcRestorationElements() << std::endl;
    std::cout << "plan total cost:" << plan.totalCost() << std::endl;

    std::cout << "ECA:" << Parallel_ECA().eval(landscape) << std::endl;

    // Helper::printLandscapeGraphviz(landscape, "test.dot");
    Helper::printInstanceGraphviz(landscape, plan, "instance_test.dot");

    const double budget = plan.totalCost() / 4;


    // Solvers::PL_ECA_3 solver;
    // solver.setLogLevel(2);
    // Solution solution = solver.solve(landscape, plan, budget);
    // std::cout << "ECA: " << Parallel_ECA().eval(Helper::decore_landscape(landscape, plan, solution)) << std::endl;
    // std::cout << "cost: " << solution.getCost() << std::endl;



    Solvers::Glutton_ECA_Inc naive_solver;
    naive_solver.setParallel(true);
    // naive_solver.setLogLevel(2);
    Solution naive_solution = naive_solver.solve(landscape, plan, budget);
    std::cout << "naive solution ECA: " << Parallel_ECA().eval(Helper::decore_landscape(landscape, plan, naive_solution)) << std::endl;
    std::cout << "naive solution cost: " << naive_solution.getCost() << std::endl;

    Helper::printSolutionGraphviz(landscape, plan, naive_solution, "solution_test.dot");

    return EXIT_SUCCESS;
}