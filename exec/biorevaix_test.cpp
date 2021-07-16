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
#include "instances_helper.hpp"

int main() {
    Instance instance = make_instance_biorevaix_level_1();
    const Landscape & landscape = instance.landscape;
    const RestorationPlan<Landscape> & plan = instance.plan;

    Helper::assert_well_formed(landscape, plan);

    std::cout << "nb nodes:" << lemon::countNodes(landscape.getNetwork()) << std::endl;
    std::cout << "nb arcs:" << lemon::countArcs(landscape.getNetwork()) << std::endl;
    std::cout << "nb options:" << plan.getNbOptions() << std::endl;
    std::cout << "nb restorable arcs:" << plan.getNbArcs() << std::endl;
    
    Helper::printInstance(instance.landscape, instance.plan, "test.eps");

    return EXIT_SUCCESS;
}