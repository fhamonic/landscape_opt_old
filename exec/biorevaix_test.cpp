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

int main(int argc, char * argv[]) {
    if(argc < 2) {
        std::cout << "Usage: " << argv[0] << " <level>" << std::endl;
        return EXIT_FAILURE;
    }
    int level = std::atoi(argv[1]);

    Instance instance = make_instance_biorevaix(level, 1, 2000);
    const Landscape & landscape = instance.landscape;

    std::cout << "nb nodes:" << lemon::countNodes(landscape.getNetwork()) << std::endl;
                    
    std::cout << "ECA level " << level << ": " << Parallel_ECA::get().eval(landscape) << std::endl;
    std::cout << "ECA complete level " << level << ": " << compute_eca_biorevaix_complete(level, 1, 2000) << std::endl;

    return EXIT_SUCCESS;
}