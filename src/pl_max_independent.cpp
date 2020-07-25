#include <iostream>

#include "csv.hpp"
#include "std_landscape_parser.hpp"

#include "pl_max_independent.hpp"

#include "lemon/graph_to_eps.h"
#include "lemon/dim2.h"

int main (int argc, const char *argv[]) {
    if(argc < 2) {
        std::cerr << "input requiered : <landscape data index file>" << std::endl;
        return -1;
    }
    std::string landscape_file = argv[1];

    Landscape * landscape = StdLandscapeParser::get().parse(landscape_file);
    const Graph_t & graph = landscape->getNetwork();


    Graph_t::NodeMap<int> node_idsMap(graph);
    Graph_t::NodeMap<bool> * independentMap = getMaxIndependentMap(graph);
    Graph_t::NodeMap<int> node_sizesMap(graph, 2);
    Graph_t::NodeMap<lemon::Color> node_colorsMap(graph, lemon::WHITE);
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        node_idsMap[v] = graph.id(v);
        if((*independentMap)[v])
            node_colorsMap[v] = lemon::BLUE;
    }

    Graph_t::ArcMap<lemon::Color> edge_colorsMap(graph, lemon::BLACK);
    for(Graph_t::ArcIt e(graph); e != lemon::INVALID; ++e) {
        if((*independentMap)[graph.u(e)] || (*independentMap)[graph.v(e)])
            edge_colorsMap[e] = lemon::YELLOW;
    }

    lemon::graphToEps(graph, "output/max_independent.eps")
            .coords(landscape->getCoordsMap())
            .nodeColors(node_colorsMap)
            .nodeSizes(node_sizesMap)
            .nodeTexts(node_idsMap)
            .nodeTextSize(0.25)
            .edgeColors(edge_colorsMap)
            .run();

    delete landscape;

    return 0;
}
