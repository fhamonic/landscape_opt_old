#include <iostream>
#include <filesystem>
#include <fstream>

#include "lemon/dijkstra.h"
#include "lemon/connectivity.h"

#include "lemon/graph_to_eps.h"

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

#include "random_chooser.hpp"

#include "helper.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

#include "catch.hpp"

#include "random_landscape_genereator.hpp"

#include "solvers/glutton_eca_inc.hpp"
#include "solvers/glutton_eca_dec.hpp"
#include "solvers/pl_eca_3.hpp"


template <typename GR, typename QM, typename PM, typename CM>
double compute_value_reversed(const concepts::AbstractLandscape<GR, QM, PM, CM> & landscape, Graph_t::Node t) {
    typedef lemon::ReverseDigraph<const Graph_t> Reversed;

    const Graph_t & original_g = landscape.getNetwork();
    Reversed reversed_g(original_g);

    lemon::MultiplicativeSimplerDijkstra<Reversed, Graph_t::ArcMap<double>> dijkstra(reversed_g, landscape.getProbabilityMap());
    double sum = 0;
    dijkstra.init(t);
    while (!dijkstra.emptyQueue()) {
        std::pair<Graph_t::Node, double> pair = dijkstra.processNextNode();
        Graph_t::Node v = pair.first;
        const double p_tv = pair.second;
        sum += landscape.getQuality(v) * p_tv;
    }
    return sum;
}

bool do_test(const Landscape & landscape, const RestorationPlan & plan, int seed) {
    Helper::assert_well_formed(landscape, plan);

    const double epsilon = 0.001;

    Solvers::Glutton_ECA_Inc glutton_eca_inc;  glutton_eca_inc.setLogLevel(0).setParallel(true);
    Solvers::Glutton_ECA_Dec glutton_eca_dec;  glutton_eca_dec.setLogLevel(0).setParallel(true);
    Solvers::PL_ECA_3 pl_eca_3;  pl_eca_3.setLogLevel(0).setNbThreads(1).setTimeout(36000);

    double max_budget = 0;
    for(RestorationPlan::Option * option : plan.options())
        max_budget += option->getCost();
    
    for(int percent_budget : {10, 20, 30}) {
        const double budget = (max_budget * percent_budget) / 100;

        Solution * glutton_inc_solution = glutton_eca_inc.solve(landscape, plan , budget);
        Solution * glutton_dec_solution = glutton_eca_dec.solve(landscape, plan , budget);
        Solution * pl_3_solution = pl_eca_3.solve(landscape, plan , budget);

        if(pl_3_solution == nullptr) {
            Helper::printSolution(landscape, std::to_string(seed), glutton_eca_dec, budget, glutton_dec_solution);
        }

        const double glutton_inc_value = ECA::get().eval(landscape, *glutton_inc_solution);
        const double glutton_dec_value = ECA::get().eval(landscape, *glutton_dec_solution);
        const double pl_3_value = ECA::get().eval(landscape, *pl_3_solution);

        if(glutton_inc_value - pl_3_value > epsilon || glutton_dec_value - pl_3_value  > epsilon) {
            std::cerr << glutton_inc_value << " " << glutton_dec_value << " " <<  pl_3_value << " " << std::endl;
            return false;
        }

        delete glutton_inc_solution;
        delete glutton_dec_solution;
        delete pl_3_solution;
    }

    return true;
}


int main() {
    const int base_seed = 98648;
    std::cout << std::setprecision(20);

    RandomInstanceGenerator instance_generator;
    const int nb_tests = 10000;
    for(int cpt_test=0; cpt_test<nb_tests; cpt_test++) {
        const int seed = base_seed + cpt_test;
        Landscape * landscape = instance_generator.generate_landscape(seed , 15, 30);
        RestorationPlan * plan = instance_generator.generate_plan(seed, *landscape, 20);
        
        if(!do_test(*landscape, *plan, seed)) {
            std::cerr << seed << std::endl;
            return EXIT_FAILURE;
        }

        std::cout << seed << " ok" << std::endl;
        
        delete plan;
        delete landscape;
    }


    return EXIT_SUCCESS;
}