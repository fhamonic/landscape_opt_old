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

Instance make_instance(const double dist_coef, const double restoration_coef) {
    Instance raw_instance =
        make_instance_biorevaix_level_2_all_troncons(restoration_coef, dist_coef);
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

    return instance;
}

int main() {
    tbb::global_control c(tbb::global_control::max_allowed_parallelism, 6);

    const double dist_coef = 1.5;
    const double restoration_coef = 6;

    Instance instance = make_instance(dist_coef, restoration_coef);
    const MutableLandscape & landscape = instance.landscape;
    RestorationPlan<MutableLandscape> & plan = instance.plan;

    Chrono chrono;

    const double eca = ECA().eval(landscape);
    // const double eca = Parallel_ECA().eval(landscape);

    std::cout << "ECA: " << eca << " in " << chrono.timeS() << " seconds"
              << std::endl;

    // Helper::printInstanceGraphviz(landscape, plan, "biorevaix.dot");

    // Solvers::Naive_ECA_Inc naive_inc;
    // naive_inc.setParallel(true);

    // const double B = plan.totalCost() / 2;

    // Solution naive_inc_solution = naive_inc.solve(landscape, plan, B);

    // std::cout << naive_inc_solution.getComputeTimeMs() << " ms" << std::endl;

    return EXIT_SUCCESS;
}