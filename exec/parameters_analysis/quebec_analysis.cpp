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
    std::ofstream data_log("output/quebec_analysis.csv");
    data_log << std::fixed << std::setprecision(6);
    data_log << "median_dist,90_eca_node_cover,90_eca_node_cover_restored,"
                "decreased_prob,base_ECA,delta_ECA"
             << std::endl;

    std::vector<double> median_dists{2000,  3000,  5000, 6000};
    std::vector<double> decreased_probs{0.75, 0.5, 0.25, 0};

    const ECA & eca = ECA();

    for(double median : median_dists) {
        for(double decreased_prob : decreased_probs) {
            Instance instance = make_instance_quebec(
                1, 0, median, decreased_prob, Point(240548, 4986893),
                Point(32360, 30000));
            const MutableLandscape & landscape = instance.landscape;
            const RestorationPlan<MutableLandscape> & plan = instance.plan;

            // Helper::assert_well_formed(landscape, plan);
            // Helper::printInstanceGraphviz(landscape, plan, "quebec.dot");

            auto restored_landscape = Helper::decore_landscape(landscape, plan);

            const double eca_90_node_cover =
                Helper::averageRatioOfNodesInECARealization(0.90, landscape);
            const double eca_90_node_cover_restored =
                Helper::averageRatioOfNodesInECARealization(0.90,
                                                            restored_landscape);

            const double base_ECA = ECA().eval(landscape);
            const double restored_ECA =
                ECA().eval(Helper::decore_landscape(landscape, plan));
            const double delta_ECA = restored_ECA - base_ECA;

            data_log << median << ',' << eca_90_node_cover * 100 << ','
                     << eca_90_node_cover_restored * 100 << ','
                     << decreased_prob << ',' << base_ECA << ',' << delta_ECA
                     << std::endl;
        }
    }

    return EXIT_SUCCESS;
}