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

Instance make_instance(const double dist_coef, const double restoration_coef) {
    Instance raw_instance =
        make_instance_biorevaix_level_2_v7(restoration_coef, dist_coef);
    Instance instance = trivial_reformulate(std::move(raw_instance));

    const MutableLandscape & landscape = instance.landscape;
    RestorationPlan<MutableLandscape> & plan = instance.plan;
    plan.initElementIDs();

    Helper::assert_well_formed(landscape, plan);
    std::cout << "nb nodes:" << lemon::countNodes(landscape.getNetwork())
              << std::endl;
    std::cout << "nb arcs:" << lemon::countArcs(landscape.getNetwork())
              << std::endl;

    int count = 0;
    for(MutableLandscape::NodeIt u(landscape.getNetwork()); u != lemon::INVALID;
        ++u)
        count += (landscape.getQuality(u) > 0 ? 1 : 0);
    std::cout << "nb nodes positive quality:" << count << std::endl;
    std::cout << "nb options:" << plan.getNbOptions() << std::endl;
    std::cout << "nb restorable arcs:" << plan.getNbArcRestorationElements()
              << std::endl;
    std::cout << "plan total cost:" << plan.totalCost() << std::endl;

    Solvers::Glutton_ECA_Dec glutton_dec_solver;
    glutton_dec_solver.setParallel(true);
    Solution budget_solution =
        glutton_dec_solver.solve(landscape, plan, plan.totalCost() * 0.3);

    auto arc_options = plan.computeArcOptionsMap();
    for(auto option : plan.options()) {
        if(budget_solution[option] > 0) continue;
        for(const auto & [a, restored_prob] : arc_options[option]) {
            plan.removeArc(option, a);
        }
    }

    return instance;
}

int main() {
    std::ofstream data_log("output/biorevaix_analysis.csv");
    data_log << std::fixed << std::setprecision(6);
    data_log
        << "dist_coef,restoration_coef,budget,budget_percent,base_ECA,max_"
           "delta_ECA,bogo_avg_delta_ECA,naive_inc_delta_ECA,naive_dec_delta_"
           "ECA,glutton_inc_delta_ECA,glutton_dec_delta_ECA,opt_delta_ECA,"
           "naive_inc_time,naive_dec_time,glutton_inc_time,glutton_dec_time,"
           "opt_time"
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
    pl_eca_3.setTimeout(36000).setLogLevel(2);

    const double dist_coef = 1.5;
    const double restoration_coef = 6;

    Instance instance = make_instance(dist_coef, restoration_coef);
    const MutableLandscape & landscape = instance.landscape;
    RestorationPlan<MutableLandscape> & plan = instance.plan;

    // Helper::printInstanceGraphviz(landscape, plan, "biorevaix.dot");

    for(double budget_percent : budget_percents) {
        const double B = plan.totalCost() * budget_percent / 100;

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

        data_log << dist_coef << ',' << restoration_coef << ',' << B << ','
                 << budget_percent << ',' << base_ECA << ',' << max_delta_ECA
                 << ',' << bogo_avg_delta_ECA << ',' << naive_inc_delta_ECA
                 << ',' << naive_dec_delta_ECA << ',' << glutton_inc_delta_ECA
                 << ',' << glutton_dec_delta_ECA << ',' << opt_delta_ECA << ','
                 << naive_inc_solution.getComputeTimeMs() << ','
                 << naive_dec_solution.getComputeTimeMs() << ','
                 << glutton_inc_solution.getComputeTimeMs() << ','
                 << glutton_dec_solution.getComputeTimeMs() << ','
                 << opt_solution.getComputeTimeMs() << std::endl;
    }

    return EXIT_SUCCESS;
}