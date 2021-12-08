#include <filesystem>
#include <fstream>
#include <iostream>

#include "indices/eca.hpp"
#include "landscape/decored_landscape.hpp"

#include "solvers/bogo.hpp"
#include "solvers/glutton_eca_dec.hpp"
#include "solvers/glutton_eca_inc.hpp"
#include "solvers/naive_eca_dec.hpp"
#include "solvers/naive_eca_inc.hpp"
#include "solvers/pl_eca_3.hpp"

#include "helper.hpp"
#include "instances_helper.hpp"
#include "print_helper.hpp"

template <typename T>
double eval(T && ls) {
    const ECA & eca = ECA();
    return std::pow(eca.eval(ls), 1);
}

int main() {
    std::ofstream data_log("output/marseille_analysis.csv");
    data_log << std::fixed << std::setprecision(6);
    data_log << "median,budget,budget_percent,base_ECA,max_delta_ECA,bogo_avg_"
                "delta_ECA,naive_inc_delta_ECA,naive_dec_delta_ECA,glutton_inc_"
                "delta_ECA,glutton_dec_delta_ECA,opt_delta_ECA,naive_inc_time,"
                "naive_dec_time,glutton_inc_time,glutton_dec_time,opt_time"
             << std::endl;

    std::vector<double> budget_percents;
    for(int i = 0; i <= 40; ++i) budget_percents.push_back(i);

    Solvers::Bogo bogo;
    bogo.setSeed(299792458);
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

    const double median = 3000;
    Instance instance = make_instance_marseille(1, 0.135, median, 100);
    instance.plan.initElementIDs();
    const MutableLandscape & landscape = instance.landscape;
    const RestorationPlan<MutableLandscape> & plan = instance.plan;

    for(double budget_percent : budget_percents) {
        const double B = plan.totalCost() * budget_percent / 100;

        Helper::printInstanceGraphviz(landscape, plan, "marseille.dot");
        Helper::printInstance(landscape, plan, "marseille.eps");

        const double base_ECA = eval(landscape);
        const double restored_ECA =
            eval(Helper::decore_landscape(landscape, plan));
        const double max_delta_ECA = restored_ECA - base_ECA;

        double bogo_avg_ECA = 0;
        {
            const int nb_bogo = 100;
            for(int i = 0; i < nb_bogo; ++i) {
                Solution bogo_solution = bogo.solve(landscape, plan, B);
                const double bogo_ECA = eval(
                    Helper::decore_landscape(landscape, plan, bogo_solution));
                bogo_avg_ECA += bogo_ECA;
            }
            bogo_avg_ECA /= nb_bogo;
        }

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

        const double bogo_avg_delta_ECA = bogo_avg_ECA - base_ECA;
        const double naive_inc_delta_ECA = naive_inc_ECA - base_ECA;
        const double naive_dec_delta_ECA = naive_dec_ECA - base_ECA;
        const double glutton_inc_delta_ECA = glutton_inc_ECA - base_ECA;
        const double glutton_dec_delta_ECA = glutton_dec_ECA - base_ECA;
        const double opt_delta_ECA = opt_ECA - base_ECA;

        data_log << median << ',' << B << ',' << budget_percent << ','
                 << base_ECA << ',' << max_delta_ECA << ','
                 << bogo_avg_delta_ECA << ',' << naive_inc_delta_ECA << ','
                 << naive_dec_delta_ECA << ',' << glutton_inc_delta_ECA << ','
                 << glutton_dec_delta_ECA << ',' << opt_delta_ECA << ','
                 << naive_inc_solution.getComputeTimeMs() << ','
                 << naive_dec_solution.getComputeTimeMs() << ','
                 << glutton_inc_solution.getComputeTimeMs() << ','
                 << glutton_dec_solution.getComputeTimeMs() << ','
                 << opt_solution.getComputeTimeMs() << std::endl;
    }

    return EXIT_SUCCESS;
}

// #include <filesystem>
// #include <fstream>
// #include <iostream>

// #include "indices/eca.hpp"
// #include "landscape/decored_landscape.hpp"

// #include "solvers/bogo.hpp"
// #include "solvers/glutton_eca_dec.hpp"
// #include "solvers/glutton_eca_inc.hpp"
// #include "solvers/pl_eca_3.hpp"

// #include "helper.hpp"
// #include "instances_helper.hpp"

// int main() {
//     std::ofstream data_log("output/marseille_analysis.csv");
//     data_log << std::fixed << std::setprecision(6);
//     data_log << "median_dist,90_eca_node_cover,90_eca_node_cover_restored,"
//                 "base_ECA,delta_ECA"
//              << std::endl;

//     std::vector<double> median_dists{500,  1000, 1500, 2000, 2500,
//                                      3000, 3500, 4000, 5000};

//     const ECA & eca = ECA();

//     for(double median : median_dists) {
//         const Instance instance = make_instance_marseille(1, 0, median, 100);
//         const MutableLandscape & landscape = instance.landscape;
//         const RestorationPlan<MutableLandscape> & plan = instance.plan;

//         // Helper::assert_well_formed(landscape, plan);
//         // Helper::printInstanceGraphviz(landscape, plan,
//         //                       "marseille.dot");

//         auto restored_landscape = Helper::decore_landscape(landscape, plan);

//         const double eca_90_node_cover =
//             Helper::averageRatioOfNodesInECARealization(0.90, landscape);
//         const double eca_90_node_cover_restored =
//             Helper::averageRatioOfNodesInECARealization(0.90,
//                                                         restored_landscape);

//         const double base_ECA = ECA().eval(landscape);
//         const double restored_ECA = ECA().eval(restored_landscape);
//         const double delta_ECA = restored_ECA - base_ECA;

//         data_log << median << ',' << eca_90_node_cover * 100 << ','
//                  << eca_90_node_cover_restored * 100 << ',' << base_ECA <<
//                  ','
//                  << delta_ECA << std::endl;
//     }

//     return EXIT_SUCCESS;
// }