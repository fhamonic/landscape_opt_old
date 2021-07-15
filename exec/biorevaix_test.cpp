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
    constexpr int level = 3;

    Instance instance = make_instance_biorevaix<level>();
    const Landscape & landscape = instance.landscape;

    Helper::assert_well_formed(landscape, instance.plan);

    std::cout << "nb nodes:" << lemon::countNodes(landscape.getNetwork()) << std::endl;
                    
    std::cout << "ECA level " << level << ": " << Parallel_ECA::get().eval(landscape) << std::endl;

    return EXIT_SUCCESS;
}