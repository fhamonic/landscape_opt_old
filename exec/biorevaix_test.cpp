#include <filesystem>
#include <fstream>
#include <iostream>

#include "lemon/connectivity.h"
#include "lemon/dijkstra.h"

#include "lemon/graph_to_eps.h"

#include "parsers/std_mutable_landscape_parser.hpp"
#include "parsers/std_restoration_plan_parser.hpp"

#include "indices/eca.hpp"
#include "indices/parallel_eca.hpp"

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
#include "print_helper.hpp"

#include "precomputation/trivial_reformulation.hpp"

#include "boost/range/algorithm/count_if.hpp"

#include "tbb/global_control.h"

int main() {
    tbb::global_control c(tbb::global_control::max_allowed_parallelism, 4);




    Instance raw_instance = make_instance_biorevaix_level_2_v7(6);
    Instance instance = trivial_reformulate(std::move(raw_instance));

    const MutableLandscape & landscape = instance.landscape;
    RestorationPlan<MutableLandscape> & plan = instance.plan;
    plan.initElementIDs();

    Helper::assert_well_formed(landscape, plan);
    std::cout << "nb nodes:" << lemon::countNodes(landscape.getNetwork())
              << std::endl;
    std::cout << "nb arcs:" << lemon::countArcs(landscape.getNetwork())
              << std::endl;

    int count = 0;
    for(MutableLandscape::NodeIt u(landscape.getNetwork()); u != lemon::INVALID;
        ++u)
        count += (landscape.getQuality(u) > 0 ? 1 : 0);
    std::cout << "nb nodes positive quality:" << count << std::endl;
    std::cout << "nb options:" << plan.getNbOptions() << std::endl;
    std::cout << "nb restorable arcs:" << plan.getNbArcRestorationElements()
              << std::endl;
    std::cout << "plan total cost:" << plan.totalCost() << std::endl;

    double intra_patch = 0;
    for(MutableLandscape::NodeIt u(landscape.getNetwork()); u != lemon::INVALID;
        ++u)
        intra_patch += landscape.getQuality(u) * landscape.getQuality(u);
    std::cout << sqrt(Parallel_ECA().eval(landscape) *
                          Parallel_ECA().eval(landscape) -
                      intra_patch)
              << std::endl;

    // Helper::printLandscapeGraphviz(landscape, "test.dot");

    Solvers::Glutton_ECA_Dec glutton_dec_solver;
    glutton_dec_solver.setParallel(true);
    Solution budget_solution =
        glutton_dec_solver.solve(landscape, plan, plan.totalCost() * 0.3);
    std::cout << "budget solution ECA: "
              << Parallel_ECA().eval(
                     Helper::decore_landscape(landscape, plan, budget_solution))
              << std::endl;
    std::cout << "in " << budget_solution.getComputeTimeMs() << "ms"
              << std::endl;

    return EXIT_SUCCESS;

    RestorationPlan<MutableLandscape> new_plan(landscape);
    auto arc_options = plan.computeArcOptionsMap();
    for(auto option : plan.options()) {
        if(budget_solution[option] == 0) continue;
        RestorationPlan<MutableLandscape>::Option new_option =
            new_plan.addOption(plan.getCost(option));
        for(const auto & [a, restored_prob] : arc_options[option]) {
            new_plan.addArc(new_option, a, restored_prob);
        }
    }

    Helper::printInstanceGraphviz(landscape, new_plan, "instance_test.dot");

    // StdMutableLandscapeParser::get().write(landscape, "", "bug");
    // StdRestorationPlanParser plan_parser(landscape);
    // plan_parser.write(plan, "", "bug");
    // return EXIT_SUCCESS;

    Solvers::Glutton_ECA_Inc glutton_inc_solver;
    Solution naive_solution =
        glutton_inc_solver.solve(landscape, new_plan, plan.totalCost() * 0.1);
    std::cout << "naive solution ECA: "
              << Parallel_ECA().eval(Helper::decore_landscape(
                     landscape, new_plan, naive_solution))
              << std::endl;
    std::cout << "in " << naive_solution.getComputeTimeMs() << "ms"
              << std::endl;

    Solvers::PL_ECA_3 solver;
    solver.setLogLevel(2);
    Solution solution =
        solver.solve(landscape, new_plan, plan.totalCost() * 0.1);
    std::cout << "ECA: "
              << Parallel_ECA().eval(
                     Helper::decore_landscape(landscape, new_plan, solution))
              << std::endl;

    return EXIT_SUCCESS;
}