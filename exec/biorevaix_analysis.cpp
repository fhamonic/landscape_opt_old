#include <filesystem>
#include <fstream>
#include <iostream>

#include "indices/eca.hpp"
#include "landscape/decored_landscape.hpp"

#include "solvers/glutton_eca_dec.hpp"
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

Instance make_instance(const double dist_coef, const double restoration_coef) {
    Instance raw_instance =
        make_instance_biorevaix_level_2_v7(restoration_coef, dist_coef);

    Helper::printInstanceGraphviz(raw_instance.landscape, raw_instance.plan,
                                  "biorevaix_raw.dot");

    Instance instance = trivial_reformulate(std::move(raw_instance));

    const MutableLandscape & landscape = instance.landscape;
    RestorationPlan<MutableLandscape> & plan = instance.plan;
    plan.initElementIDs();

    Helper::assert_well_formed(landscape, plan);
    // std::cout << "nb nodes:" << lemon::countNodes(landscape.getNetwork())
    //           << std::endl;
    // std::cout << "nb arcs:" << lemon::countArcs(landscape.getNetwork())
    //           << std::endl;

    // int count = 0;
    // for(MutableLandscape::NodeIt u(landscape.getNetwork()); u !=
    // lemon::INVALID;
    //     ++u)
    //     count += (landscape.getQuality(u) > 0 ? 1 : 0);
    // std::cout << "nb nodes positive quality:" << count << std::endl;
    // std::cout << "nb options:" << plan.getNbOptions() << std::endl;
    // std::cout << "nb restorable arcs:" << plan.getNbArcRestorationElements()
    //           << std::endl;
    // std::cout << "plan total cost:" << plan.totalCost() << std::endl;

    Solvers::Glutton_ECA_Dec glutton_dec_solver;
    glutton_dec_solver.setParallel(true);
    Solution budget_solution =
        glutton_dec_solver.solve(landscape, plan, plan.totalCost() * 0.3);

    auto arc_options = plan.computeArcOptionsMap();
    int nb_options = plan.getNbOptions();
    for(auto option : plan.options()) {
        if(budget_solution[option] > 0) continue;
        --nb_options;
        for(const auto & [a, restored_prob] : arc_options[option]) {
            plan.removeArc(option, a);
        }
    }

    std::cout << "nb nodes:" << lemon::countNodes(landscape.getNetwork())
              << std::endl;
    std::cout << "nb arcs:" << lemon::countArcs(landscape.getNetwork())
              << std::endl;
    int count = 0;
    for(MutableLandscape::NodeIt u(landscape.getNetwork()); u != lemon::INVALID;
        ++u)
        count += (landscape.getQuality(u) > 0 ? 1 : 0);
    std::cout << "nb nodes positive quality:" << count << std::endl;
    std::cout << "nb options:" << nb_options << std::endl;
    std::cout << "nb restorable arcs:" << plan.getNbArcRestorationElements()
              << std::endl;
    std::cout << "plan total cost:" << plan.totalCost() << std::endl;

    return instance;
}

int main() {
    std::ofstream data_log("output/biorevaix_analysis.csv");
    data_log << std::fixed << std::setprecision(6);
    data_log << "budget,budget_percent,base_ECA,max_delta_ECA,pl_eca_2_ECA,pl_"
                "eca_2_obj,pl_eca_2_variables,pl_2_constraints,pl_2_entries,pl_"
                "2_time,pl_eca_3_ECA,pl_eca_3_obj,pl_eca_3_variables,pl_3_"
                "constraints,pl_3_entries,pl_3_preprocessing_time,pl_3_time"
             << std::endl;

    std::vector<double> budget_percents;
    for(int i = 0; i <= 40; ++i) budget_percents.push_back(i);

    Solvers::PL_ECA_2 pl_eca_2;
    pl_eca_2.setTimeout(36000).setLogLevel(2);
    Solvers::PL_ECA_3 pl_eca_3;
    pl_eca_3.setTimeout(36000).setLogLevel(2);

    const double dist_coef = 1.5;
    const double restoration_coef = 6;

    Instance instance = make_instance(dist_coef, restoration_coef);
    const MutableLandscape & landscape = instance.landscape;
    RestorationPlan<MutableLandscape> & plan = instance.plan;

    // Helper::printInstanceGraphviz(landscape, plan, "biorevaix_instance.dot");

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