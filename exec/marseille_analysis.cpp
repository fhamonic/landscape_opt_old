#include <filesystem>
#include <fstream>
#include <iostream>

#include "indices/eca.hpp"
#include "landscape/decored_landscape.hpp"

#include "solvers/pl_eca_2.hpp"
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
    data_log << "budget,budget_percent,base_ECA,max_delta_ECA,pl_eca_2_ECA,pl_"
                "eca_2_obj,pl_eca_2_variables,pl_2_constraints,pl_2_entries,pl_"
                "2_time,pl_eca_3_ECA,pl_eca_3_obj,pl_eca_3_variables,pl_3_"
                "constraints,pl_3_entries,pl_3_preprocessing_time,pl_3_time"
             << std::endl;

    std::vector<double> budget_percents;
    for(int i = 0; i <= 40; ++i) budget_percents.push_back(i);

    Solvers::PL_ECA_2 pl_eca_2;
    pl_eca_2.setTimeout(3600).setLogLevel(2);
    Solvers::PL_ECA_3 pl_eca_3;
    pl_eca_3.setTimeout(3600).setLogLevel(2);

    const double median = 3000;
    Instance instance = make_instance_marseille(1, 0.135, median, 100);
    instance.plan.initElementIDs();
    const MutableLandscape & landscape = instance.landscape;
    const RestorationPlan<MutableLandscape> & plan = instance.plan;

    
    for(double budget_percent : budget_percents) {
        const double B = plan.totalCost() * budget_percent / 100;

        const double base_ECA = eval(landscape);
        const double restored_ECA =
            eval(Helper::decore_landscape(landscape, plan));
        const double max_delta_ECA = restored_ECA - base_ECA;

        Solution pl_2_solution = pl_eca_2.solve(landscape, plan, B);
        Solution pl_3_solution = pl_eca_3.solve(landscape, plan, B);

        const double pl_2_ECA =
            eval(Helper::decore_landscape(landscape, plan, pl_2_solution));
        const double pl_3_ECA =
            eval(Helper::decore_landscape(landscape, plan, pl_3_solution));

        data_log << B << ',' << budget_percent << ',' << base_ECA << ','
                 << max_delta_ECA << ',' << pl_2_ECA << ',' << pl_2_solution.obj
                 << ',' << pl_2_solution.nb_vars << ','
                 << pl_2_solution.nb_constraints << ','
                 << pl_2_solution.nb_elems << ','
                 << pl_2_solution.getComputeTimeMs() << ',' << pl_3_ECA << ','
                 << pl_3_solution.obj << ',' << pl_3_solution.nb_vars << ','
                 << pl_3_solution.nb_constraints << ','
                 << pl_3_solution.nb_elems << ','
                 << pl_3_solution.preprocessing_time << ','
                 << pl_3_solution.getComputeTimeMs() << std::endl;
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