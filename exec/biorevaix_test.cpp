#include <filesystem>
#include <fstream>
#include <iostream>

#include "lemon/connectivity.h"
#include "lemon/dijkstra.h"

#include "lemon/graph_to_eps.h"

#include "parsers/std_mutable_landscape_parser.hpp"
#include "parsers/std_restoration_plan_parser.hpp"

#include "indices/eca.hpp"
#include "indices/parallel_eca.hpp"

#include "landscape/decored_landscape.hpp"

#include "utils/random_chooser.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

#include "solvers/bogo.hpp"
#include "solvers/glutton_eca_dec.hpp"
#include "solvers/glutton_eca_inc.hpp"
#include "solvers/naive_eca_dec.hpp"
#include "solvers/naive_eca_inc.hpp"
#include "solvers/pl_eca_2.hpp"
#include "solvers/pl_eca_3.hpp"
#include "solvers/randomized_rounding.hpp"

#include "helper.hpp"
#include "instances_helper.hpp"
#include "print_helper.hpp"

#include "precomputation/trivial_reformulation.hpp"

#include "boost/range/algorithm/count_if.hpp"

#include "tbb/global_control.h"

Instance make_instance(const double dist_coef, const double restoration_coef) {
    //*
    Instance raw_instance = make_instance_biorevaix_level_1_all_troncons_updated(
        restoration_coef, dist_coef);
    Instance instance = trivial_reformulate(std::move(raw_instance));
    /*/
    Instance instance = make_instance_biorevaix_level_2_all_troncons(
        restoration_coef, dist_coef);
    //*/

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

    return instance;
}

int main() {
    const int from_option = 0;
    const int to_option = 2928;


    const double dist_coef = 1.5;
    const double restoration_coef = 6;

    Instance instance = make_instance(dist_coef, restoration_coef);
    const MutableLandscape & landscape = instance.landscape;
    RestorationPlan<MutableLandscape> & plan = instance.plan;

    Chrono chrono;

    // const double eca = ECA().eval(landscape);
    const double prec_eca = Parallel_ECA().eval(landscape);

    std::cout << "prec ECA: " << prec_eca << " in " << chrono.timeMs() << " ms"
              << std::endl;

    const auto nodeOptions = plan.computeNodeOptionsMap();
    const auto arcOptions = plan.computeArcOptionsMap();

    DecoredLandscape<MutableLandscape> decored_landscape(landscape);
    for(auto option : plan.options()) {
        if(!(from_option < option && option < to_option)) continue;
        decored_landscape.reset();
        decored_landscape.apply(nodeOptions[option], arcOptions[option]);

        const double eca = Parallel_ECA().eval(decored_landscape);
        const double delta_eca = (eca - prec_eca);
        const double ratio = delta_eca / plan.getCost(option);

        std::cout << "option " << option << " delta " << delta_eca << " ratio " << ratio << std::endl;
    }





    // io::CSVReader<3> options("log");
    // options.read_header(io::ignore_extra_column, "option", "delta", "ratio");
    // io::CSVReader<2> troncons("log copy");
    // troncons.read_header(io::ignore_extra_column, "option", "troncon");

    // int option;
    // double delta, ratio;
    // int option_2, troncon;
    // while(options.read_row(option, delta, ratio)) {
    //     troncons.read_row(option_2, troncon);

    //     if(option != option_2) {
    //         std::cout << "caca !" << std::endl;
    //         break;
    //     };
    //     if(delta < 0.01) continue;

    //     std::cout << std::fixed << troncon << "," << delta << "," << ratio << std::endl;
    // }

    return EXIT_SUCCESS;
}