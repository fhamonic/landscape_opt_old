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

std::pair<MutableLandscape, RestorationPlan<MutableLandscape>> make_instance() {
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

    return std::make_pair(landscape, new_plan);
}

int main() {
    tbb::global_control c(tbb::global_control::max_allowed_parallelism, 8);

    auto p = make_instance();
    const MutableLandscape & landscape = p.landscape;
    RestorationPlan<MutableLandscape> & plan = p.plan;

    Helper::printInstanceGraphviz(landscape, plan, "biorevaix.dot");


   std::vector<double> budget_percents;
    for(int i = 0; i <= 40; ++i) budget_percents.push_back(i);

    Solvers::Naive_ECA_Inc naive_inc;
    naive_inc.setParallel(true);
    Solvers::Naive_ECA_Dec naive_dec;
    naive_dec.setParallel(true);
    Solvers::Glutton_ECA_Inc glutton_inc;
    glutton_inc.setParallel(true);
    Solvers::Glutton_ECA_Dec glutton_dec;
    glutton_dec.setParallel(true);
    Solvers::PL_ECA_3 pl_eca_3;
    pl_eca_3.setLogLevel(2);

    const Instance instance = make_instance_quebec(
        1, 0, median, 0, Point(240548, 4986893), Point(32360, 30000));
    const MutableLandscape & landscape = instance.landscape;
    const RestorationPlan<MutableLandscape> & plan = instance.plan;  

    for(double budget_percent : budget_percents) {      
        const double B = plan.totalCost() * budget_percent / 100;

        // Helper::printInstanceGraphviz(landscape, plan, "quebec.dot");
        // Helper::printInstance(landscape, plan, "quebec.eps");

        const double base_ECA = eval(landscape);
        const double restored_ECA =
            eval(Helper::decore_landscape(landscape, plan));
        const double max_delta_ECA = restored_ECA - base_ECA;

        Solution naive_inc_solution = naive_inc.solve(landscape, plan, B);
        Solution naive_dec_solution = naive_dec.solve(landscape, plan, B);
        Solution glutton_inc_solution = glutton_inc.solve(landscape, plan, B);
        Solution glutton_dec_solution = glutton_dec.solve(landscape, plan, B);
        Solution opt_solution = pl_eca_3.solve(landscape, plan, B);

        const double naive_inc_ECA =
            eval(Helper::decore_landscape(landscape, plan, naive_inc_solution));
        const double naive_dec_ECA =
            eval(Helper::decore_landscape(landscape, plan, naive_dec_solution));
        const double glutton_inc_ECA = eval(
            Helper::decore_landscape(landscape, plan, glutton_inc_solution));
        const double glutton_dec_ECA = eval(
            Helper::decore_landscape(landscape, plan, glutton_dec_solution));
        const double opt_ECA =
            eval(Helper::decore_landscape(landscape, plan, opt_solution));

        const double naive_inc_delta_ECA = naive_inc_ECA - base_ECA;
        const double naive_dec_delta_ECA = naive_dec_ECA - base_ECA;
        const double glutton_inc_delta_ECA = glutton_inc_ECA - base_ECA;
        const double glutton_dec_delta_ECA = glutton_dec_ECA - base_ECA;
        const double opt_delta_ECA = opt_ECA - base_ECA;

        data_log << median << ',' << B << ',' << budget_percent << ','
                 << base_ECA << ',' << max_delta_ECA << ','
                 << naive_inc_delta_ECA << ',' << naive_dec_delta_ECA << ','
                 << glutton_inc_delta_ECA << ',' << glutton_dec_delta_ECA << ','
                 << opt_delta_ECA << std::endl;
    }

    return EXIT_SUCCESS;
}