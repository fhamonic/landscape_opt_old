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

RestorationPlan<Landscape>::Option* find_option(const RestorationPlan<Landscape>& plan, const int id) {
    for(RestorationPlan<Landscape>::Option* option : plan.options())
        if(option->getId() == id)
            return option;
    return nullptr;
}


int main (int argc, const char *argv[]) {
    if(argc < 3) {
        std::cerr << "input requiered : <landscape_file> <plan_file>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];
    std::filesystem::path plan_path = argv[2];


    const int seed = 1245;
    std::cout << std::setprecision(10);

    
    Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);
    const Graph_t & graph = landscape->getNetwork();
    StdRestorationPlanParser parser(*landscape);
    RestorationPlan<Landscape>* plan = parser.parse(plan_path);
    
    Helper::assert_well_formed(*landscape, *plan);


    std::chrono::time_point<std::chrono::high_resolution_clock> t0, t1;
    MyContractionAlgorithm algo;

    t0 = std::chrono::high_resolution_clock::now();
    Graph_t::NodeMap<ContractionResult> * results = algo.precompute(*landscape, *plan);
    t1 = std::chrono::high_resolution_clock::now();

    const int n = lemon::countNodes(graph);
    const double epsilon = n * std::numeric_limits<double>::epsilon();

    for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
        ContractionResult result = (*results)[t];
        const double base = compute_value_reversed(*landscape, t);
        const double contracted = compute_value_reversed(*result.landscape, result.t);
        
        if(fabs(base - contracted) > epsilon) {
            std::cout << graph.id(t) << " : " << base << " != " << contracted << std::endl;
        }
    }


    const int nb_options = plan->getNbOptions();
    RandomChooser<int> option_chooser(seed);
    for(int i=0; i<nb_options; ++i)
        option_chooser.add(i, 1);

    std::default_random_engine gen;
    gen.seed(seed+1);
    std::uniform_int_distribution<> dis(0, nb_options);

    for(int i=0; i<1000; ++i) {
        std::vector<int> picked_options;
        double nb_picked_options = dis(gen);
        option_chooser.reset();
        for(int j=0; j<nb_picked_options; ++j)
            picked_options.push_back(option_chooser.pick());

        DecoredLandscape decored_landscape(*landscape);
        for(int option_id : picked_options)
            decored_landscape.apply(find_option(*plan, option_id));


        for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
            ContractionResult result = (*results)[t];

            DecoredLandscape decored_contracted_landscape(*result.landscape);
            for(int option_id : picked_options)
                decored_landscape.apply(find_option(*result.plan, option_id));

            const double base = compute_value_reversed(decored_landscape, t);
            const double contracted = compute_value_reversed(decored_contracted_landscape, result.t);
            
            if(fabs(base - contracted) > epsilon) {
                std::cout << graph.id(t) << " : " << base << " != " << contracted << std::endl;
            }
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

    int normal_time_us = std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count();
    std::cout << normal_time_us << std::endl;


    delete results;
    delete landscape;


    return EXIT_SUCCESS;
}
