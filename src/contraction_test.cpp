#include <iostream>
#include <filesystem>
#include <fstream>

#include "lemon/dijkstra.h"
#include "lemon/connectivity.h"

#include "lemon/graph_to_eps.h"

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

#include "utils/random_chooser.hpp"

#include "helper.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

#include "catch.hpp"
#include "utils/random_instance_generator.hpp"


#include "instances_helper.hpp"


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

void do_test(const Landscape & landscape, const RestorationPlan & plan, int seed) {
    Helper::assert_well_formed(landscape, plan);
    const Graph_t & graph = landscape.getNetwork();

    // const int nb_nodes = lemon::countNodes(graph);
    const double epsilon_n = 1e-6;
    const double epsilon_n2 = 1e-5;

    Chrono chrono;
    MyContractionAlgorithm algo;

    Graph_t::NodeMap<ContractionResult> * results = algo.precompute(landscape, plan);
    int precompute_time_ms = chrono.lapTimeMs();

    for(Graph_t::NodeIt t(graph); t!=lemon::INVALID; ++t) {
        const double base = compute_value_reversed(landscape, t);
                
        ContractionResult result = (*results)[t];
        Helper::assert_well_formed(*result.landscape, *result.plan);
        const double contracted = compute_value_reversed(*result.landscape, result.t);
        
        if(fabs(base - contracted) > epsilon_n2) {
            std::cout << "base " << graph.id(t) << " : " << base << " != " << contracted << std::endl;
        }
    }

    RandomChooser<RestorationPlan::Option> option_chooser(seed);
    for(int i=0; i<plan.getNbOptions(); ++i)
        option_chooser.add(i, 1);

    std::default_random_engine gen;
    gen.seed(seed+1);
    std::uniform_int_distribution<> dis(0, plan.getNbOptions());

    const int nb_tests = 1000;

    for(int i=0; i<nb_tests; ++i) {
        std::vector<RestorationPlan::Option> picked_options;
        double nb_picked_options = dis(gen);
        option_chooser.reset();
        for(int j=0; j<nb_picked_options; ++j)
            picked_options.push_back(option_chooser.pick());

        DecoredLandscape decored_landscape(landscape);
        for(RestorationPlan::Option option : picked_options)
            decored_landscape.apply(plan, option);

        double sum_base = 0;
        double sum_contracted = 0;
        for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
            const double base = landscape.getQuality(t) * compute_value_reversed(decored_landscape, t);

            ContractionResult result = (*results)[t];
            DecoredLandscape decored_contracted_landscape(*result.landscape);
            for(RestorationPlan::Option option : picked_options)
                decored_contracted_landscape.apply(*result.plan, option);
            const double contracted = landscape.getQuality(t) * compute_value_reversed(decored_contracted_landscape, result.t);
            
            if(fabs(base - contracted) > epsilon_n) {
                std::cout << "test " << i << " source " << graph.id(t) << " : " << base << " != " << contracted << std::endl;
            }

            sum_base += base;
            sum_contracted += contracted;
        }

        if(fabs(sum_base - sum_contracted) > epsilon_n2) {
            std::cout << "test " << i << " sum : " << sum_base << " != " << sum_contracted << std::endl;
        }
    }

    int sum_of_nb_nodes = 0;
    int sum_of_nb_arcs = 0;
    for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
        ContractionResult result = (*results)[t];
        sum_of_nb_nodes += lemon::countNodes(result.landscape->getNetwork());   
        sum_of_nb_arcs += lemon::countArcs(result.landscape->getNetwork());
    }
    std::cout << "Total nb of nodes : " << sum_of_nb_nodes << std::endl;
    std::cout << "Total nb of arcs : " << sum_of_nb_arcs << std::endl;
    std::cout << "time : " << precompute_time_ms << std::endl;

    // int normal_time_us = std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count();
    // std::cout << normal_time_us << std::endl;

    delete results;
}


int main() {
    // const int nb_nodes = 20;
    // const int nb_arcs = 200;
    // const int nb_options = 50;
    // const bool restore_nodes = true;

    const int seed = 765;
    std::cout << std::setprecision(20);

    RandomInstanceGenerator instance_generator;
    const int nb_tests = 1;
    for(int cpt_test=0; cpt_test<nb_tests; cpt_test++) {
        Instance * instance = make_instance_quebec(1.0, 0.01, 2800, true, true);
        //Instance * instance = make_instance_marseille(2.0, 0.01, 1400, true, true);
        
        do_test(instance->landscape, instance->plan, seed + cpt_test);
        
        // const Landscape & landscape = instance->landscape;
        // const RestorationPlan & plan = instance->plan;
        
        // Landscape * landscape = instance_generator.generate_landscape(seed + cpt_test, nb_nodes, nb_arcs, false);
        // RestorationPlan * plan = instance_generator.generate_plan(seed + cpt_test, *landscape, nb_options, restore_nodes);
        
        // Helper::assert_well_formed(landscape, plan);

        // do_test(*landscape, *plan, seed + cpt_test);
        // delete plan;
        // delete landscape;

        // return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}
