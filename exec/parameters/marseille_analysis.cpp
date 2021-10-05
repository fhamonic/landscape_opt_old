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

int main() {
    std::ofstream data_log("output/marseille_analysis.csv");
    data_log << std::fixed << std::setprecision(6);
    data_log << "median_dist,90_eca_node_cover,90_eca_node_cover_restored,"
                "base_ECA,delta_ECA"
             << std::endl;

    std::vector<double> median_dists{500,  1000, 1500, 2000, 2500,
                                     3000, 3500, 4000, 5000};

    const ECA & eca = ECA();

    for(double median : median_dists) {
        const Instance instance = make_instance_marseille(1, 0, median, 100);
        const MutableLandscape & landscape = instance.landscape;
        const RestorationPlan<MutableLandscape> & plan = instance.plan;

        // Helper::assert_well_formed(landscape, plan);
        // Helper::printInstanceGraphviz(landscape, plan,
        //                       "marseille.dot");

        auto restored_landscape = Helper::decore_landscape(landscape, plan);

        const double eca_90_node_cover =
            Helper::averageRatioOfNodesInECARealization(0.90, landscape);
        const double eca_90_node_cover_restored =
            Helper::averageRatioOfNodesInECARealization(0.90,
                                                        restored_landscape);

        const double base_ECA = ECA().eval(landscape);
        const double restored_ECA = ECA().eval(restored_landscape);
        const double delta_ECA = restored_ECA - base_ECA;

        data_log << median << ',' << eca_90_node_cover * 100 << ','
                 << eca_90_node_cover_restored * 100 << ',' << base_ECA << ','
                 << delta_ECA << std::endl;
    }

    return EXIT_SUCCESS;
}