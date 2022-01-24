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
    std::ofstream data_log("output/aude_analysis.csv");
    data_log << std::fixed << std::setprecision(6);
    data_log
        << "median,restored_prob,budget,budget_percent,base_ECA,max_delta_"
           "ECA,bogo_avg_delta_ECA,naive_inc_delta_ECA,naive_dec_delta_ECA,"
           "glutton_inc_delta_ECA,glutton_dec_delta_ECA,opt_delta_ECA,naive_"
           "inc_time,naive_dec_time,glutton_inc_time,glutton_dec_time,opt_time"
        << std::endl;

    std::vector<double> budget_values;
    for(int i = 0; i <= 15; ++i) budget_values.push_back(i);

    Solvers::Bogo bogo;
    bogo.setSeed(299792458);
    Solvers::Naive_ECA_Inc naive_inc;
    Solvers::Naive_ECA_Dec naive_dec;
    Solvers::Glutton_ECA_Inc glutton_inc;
    Solvers::Glutton_ECA_Dec glutton_dec;
    Solvers::PL_ECA_3 pl_eca_3;

    const double median = 300;
    const double restored_prob = 0.8;
    Instance instance = make_instance_aude(median, restored_prob);
    instance.plan.initArcElementIDs();
    const MutableLandscape & landscape = instance.landscape;
    const RestorationPlan<MutableLandscape> & plan = instance.plan;

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
    // Helper::printInstanceGraphviz(landscape, plan, "aude.dot");
    // Helper::printInstance(landscape, plan, "aude.eps");

    for(double B : budget_values) {
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

        data_log << median << ',' << restored_prob << ',' << B << ','
                 << B / plan.totalCost() * 100 << ',' << base_ECA << ','
                 << max_delta_ECA << ',' << bogo_avg_delta_ECA << ','
                 << naive_inc_delta_ECA << ',' << naive_dec_delta_ECA << ','
                 << glutton_inc_delta_ECA << ',' << glutton_dec_delta_ECA << ','
                 << opt_delta_ECA << ','
                 << naive_inc_solution.getComputeTimeMs() << ','
                 << naive_dec_solution.getComputeTimeMs() << ','
                 << glutton_inc_solution.getComputeTimeMs() << ','
                 << glutton_dec_solution.getComputeTimeMs() << ','
                 << opt_solution.getComputeTimeMs() << std::endl;
    }

    return EXIT_SUCCESS;
}