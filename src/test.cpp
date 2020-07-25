#include <iostream>
#include <filesystem>
#include <fstream>

#include "lemon/dijkstra.h"
#include "algorithms/multiplicative_dijkstra.h"

#include "indices/eca.hpp"

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"



#include "algorithms/identify_strong_arcs.h"


template <typename GR, typename QM, typename DM, typename CM>
double eval1(const concepts::AbstractLandscape<GR, QM, DM, CM> & landscape) {
    const GR & g = landscape.getNetwork();
    const QM & qualityMap = landscape.getQualityMap();
    const DM & difficultyMap = landscape.getProbabilityMap();
    
    lemon::Dijkstra<GR, DM> dijkstra(g, difficultyMap);
       
    double sum = 0;

    for (typename GR::NodeIt s(g); s != lemon::INVALID; ++s) {
        dijkstra.run(s);
        for (typename GR::NodeIt t(g); t != lemon::INVALID; ++t) {
            if(! dijkstra.reached(t)) continue;

            const double p_st = std::exp(-dijkstra.dist(t));

            sum += qualityMap[s] * qualityMap[t] * p_st;
        }
    }
    
    return std::sqrt(sum);
}

template <typename GR, typename QM, typename DM, typename CM>
double eval2(const concepts::AbstractLandscape<GR, QM, DM, CM> & landscape) {
    const GR & g = landscape.getNetwork();
    const QM & qualityMap = landscape.getQualityMap();
    const DM & probabilityMap = landscape.getProbabilityMap();
    
    lemon::MultiplicativeSimplerDijkstra<GR, DM> dijkstra(g, probabilityMap);
    double sum = 0;
    for (typename GR::NodeIt s(g); s != lemon::INVALID; ++s) {
        dijkstra.init(s);

        while (!dijkstra.emptyQueue()) {
            std::pair<typename GR::Node, double> pair = dijkstra.processNextNode();
            Graph_t::Node t = pair.first;
            const double p_st = pair.second;
            sum += qualityMap[s] * qualityMap[t] * p_st;
        }
    }
    return std::sqrt(sum);
}


int main (int argc, const char *argv[]) {
    if(argc < 2) {
        std::cerr << "input requiered : <landscape_file>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];

    const double alpha = 2000;
    
    Landscape * landscape1 = StdLandscapeParser::get().parse(landscape_path);
    Landscape * landscape2 = StdLandscapeParser::get().parse(landscape_path);
    const Graph_t & graph = landscape1->getNetwork();

    std::chrono::time_point<std::chrono::high_resolution_clock> t0, t1, t2, t3;

    for(Graph_t::ArcIt b(graph); b != lemon::INVALID; ++b)
        landscape1->setProbability(b, -std::log(landscape2->getProbability(b)));

    t0 = std::chrono::high_resolution_clock::now();
    double normal_eca = eval1(*landscape1);
    t1 = std::chrono::high_resolution_clock::now();

    t2 = std::chrono::high_resolution_clock::now();
    double new_eca = eval2(*landscape2);
    t3 = std::chrono::high_resolution_clock::now();



    std::cout << std::setprecision(10);

    int normal_time_us = std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count();
    int new_time_us = std::chrono::duration_cast<std::chrono::microseconds>(t3-t2).count();

    std::cout << normal_eca << " " << new_eca << std::endl;
    std::cout << normal_time_us << " " << new_time_us << std::endl;

    

    return EXIT_SUCCESS;
}        