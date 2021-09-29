#include <filesystem>
#include <fstream>
#include <iostream>

#include "indices/eca.hpp"
#include "landscape/decored_landscape.hpp"

#include "solvers/bogo.hpp"
#include "solvers/glutton_eca_dec.hpp"
#include "solvers/glutton_eca_inc.hpp"
#include "solvers/pl_eca_3.hpp"

#include "helper.hpp"
#include "instances_helper.hpp"
#include "print_helper.hpp"

int main() {
    std::ofstream data_log("output/biorevaix_analysis.csv");
    data_log << std::fixed << std::setprecision(6);
    data_log << "distance_coef,90_eca_node_cover,90_eca_node_cover_restored,"
                "restoration_coef,base_ECA,delta_ECA,delta_ECA_percent"
             << std::endl;

    std::vector<double> distance_coefs{1, 1.25, 1.5, 1.75, 2};
    std::vector<double> restoration_coefs{3, 4, 5, 6, 7, 8};

    const ECA & eca = ECA();

    for(double distance_coef : distance_coefs) {
        for(double restoration_coef : restoration_coefs) {
            Instance instance = make_instance_biorevaix_level_2_v7(restoration_coef, 1 / distance_coef);
            const MutableLandscape & landscape = instance.landscape;
            const RestorationPlan<MutableLandscape> & plan = instance.plan;

            Helper::assert_well_formed(landscape, plan);
            Helper::printInstanceGraphviz(landscape, plan,
                                  "biorevaix.dot");

            std::cout << lemon::countNodes(landscape.getNetwork()) << std::endl;

            auto restored_landscape = Helper::decore_landscape(landscape, plan);

            const double eca_90_node_cover =
                Helper::averageRatioOfNodesInECARealization(0.90, landscape);
            const double eca_90_node_cover_restored =
                Helper::averageRatioOfNodesInECARealization(0.90,
                                                            restored_landscape);

            const double base_ECA = ECA().eval(landscape);
            const double restored_ECA = ECA().eval(restored_landscape);
            const double delta_ECA = restored_ECA - base_ECA;

            data_log << distance_coef << ',' << eca_90_node_cover * 100 << ','
                     << eca_90_node_cover_restored * 100 << ',' << restoration_coef
                     << ',' << base_ECA << ',' << delta_ECA << ',' << delta_ECA / base_ECA * 100 << std::endl;
        }
    }

    return EXIT_SUCCESS;
}