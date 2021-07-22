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
    Instance instance = make_instance_biorevaix_level_1(10, Point(897286.5,6272835.5), 400);
    const Landscape & landscape = instance.landscape;
    RestorationPlan<Landscape> & plan = instance.plan;

    Helper::assert_well_formed(landscape, plan);

    std::cout << "nb nodes:" << lemon::countNodes(landscape.getNetwork()) << std::endl;
    std::cout << "nb arcs:" << lemon::countArcs(landscape.getNetwork()) << std::endl;
        
    int count = 0;
    for(Graph_t::NodeIt u(landscape.getNetwork()); u != lemon::INVALID; ++u)
        count += (landscape.getQuality(u)>0 ? 1 : 0);    
    std::cout << "nb nodes positive quality:" << count << std::endl;
    std::cout << "nb options:" << plan.getNbOptions() << std::endl;
    std::cout << "nb restorable arcs:" << plan.getNbArcRestorationElements() << std::endl;

    std::cout << "ECA:" << Parallel_ECA().eval(landscape) << std::endl;

    Helper::printLandscapeGraphviz(landscape, "test.dot");

    Solvers::PL_ECA_3 pl_eca_3;
    pl_eca_3.setLogLevel(2);

    plan.initElementIDs();
    Solution solution = pl_eca_3.solve(landscape, plan, 2);

    std::cout << "cost: " << solution.getCost() << std::endl;

    return EXIT_SUCCESS;
}