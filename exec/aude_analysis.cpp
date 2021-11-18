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
    data_log << "median,restored_prob,budget,budget_percent,base_ECA,max_delta_"
                "ECA,naive_inc_delta_ECA,naive_dec_delta_ECA,glutton_inc_delta_"
                "ECA,glutton_dec_delta_ECA,opt_delta_ECA"
             << std::endl;


    std::vector<double> budget_values;
    for(int i = 0; i <= 30; ++i) budget_values.push_back(i);

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

    // Helper::printInstanceGraphviz(landscape, plan, "aude.dot");
    // Helper::printInstance(landscape, plan, "aude.eps");

    for(double B : budget_values) {
        const double base_ECA = eval(landscape);
        const double restored_ECA =
            eval(Helper::decore_landscape(landscape, plan));
        const double max_delta_ECA = restored_ECA - base_ECA;

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

        const double naive_inc_delta_ECA = naive_inc_ECA - base_ECA;
        const double naive_dec_delta_ECA = naive_dec_ECA - base_ECA;
        const double glutton_inc_delta_ECA = glutton_inc_ECA - base_ECA;
        const double glutton_dec_delta_ECA = glutton_dec_ECA - base_ECA;
        const double opt_delta_ECA = opt_ECA - base_ECA;

        data_log << median << ',' << restored_prob << ',' << B << ','
                 << B / plan.totalCost() * 100 << ',' << base_ECA << ','
                 << max_delta_ECA << ',' << naive_inc_delta_ECA << ','
                 << naive_dec_delta_ECA << ',' << glutton_inc_delta_ECA << ','
                 << glutton_dec_delta_ECA << ',' << opt_delta_ECA << std::endl;
    }

    return EXIT_SUCCESS;
}