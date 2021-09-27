#include <filesystem>
#include <fstream>
#include <iostream>

#include "lemon/connectivity.h"
#include "lemon/dijkstra.h"

#include "lemon/graph_to_eps.h"

#include "parsers/std_mutable_landscape_parser.hpp"
#include "parsers/std_restoration_plan_parser.hpp"

#include "indices/eca.hpp"
#include "landscape/decored_landscape.hpp"

#include "utils/random_chooser.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

#include "solvers/bogo.hpp"
#include "solvers/glutton_eca_dec.hpp"
#include "solvers/glutton_eca_inc.hpp"
#include "solvers/pl_eca_3.hpp"

#include "helper.hpp"
#include "instances_helper.hpp"
#include "print_helper.hpp"

int main() {
    std::vector<std::unique_ptr<concepts::Solver>> solvers =
        construct_solvers();

    std::ofstream data_log("output/data.log");
    data_log << std::fixed << std::setprecision(6);
    data_log << "median_dist "
             << "restored_probs "
             << "delta_ECA " << std::endl;

    std::vector<double> median_dists{900};
    std::vector<double> restored_probs{0.4, 0.5, 0.6, 0.8, 0.9, 1};

    const ECA & eca = ECA();

    for(double median : median_dists) {
        for(double restored_prob : restored_probs) {
            Instance instance = make_instance_quebec(median, restored_prob);
            const MutableLandscape & landscape = instance.landscape;
            const RestorationPlan<MutableLandscape> & plan = instance.plan;

            Helper::assert_well_formed(landscape, plan);
            Helper::printInstance(landscape, plan,
                                  "quebec-(" + std::to_string(orig.x) + "," +
                                      std::to_string(orig.y) + ").eps");

            const double base_ECA = ECA().eval(landscape);
            const double restored_ECA =
                ECA().eval(Helper::decore_landscape(landscape, plan));
            const double delta_ECA = restored_ECA - base_ECA;

            data_log << median << " " << restored_prob << " "
                     << delta_ECA << std::endl;
        }
    }

    return EXIT_SUCCESS;
}