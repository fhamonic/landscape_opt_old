#ifndef TRIVIAL_REFORMULATION_HPP
#define TRIVIAL_REFORMULATION_HPP

#include "landscape/landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"

#include <lemon/connectivity.h>

std::pair<Landscape, RestorationPlan<Landscape>> trivial_reformulate(Landscape & landscape, const RestorationPlan<Landscape> & plan) {
    using Graph = Landscape::Graph;
    using ArcFilterMap = Graph::ArcMap<bool>;
    using ComponentMap = Graph::NodeMap<int>;

    const Graph & graph = landscape.getNetwork();
    ArcFilterMap arc_filter(graph, false);
    for(Graph::ArcIt a(graph); a!=lemon::INVALID; ++a)
        arc_filter[a] = landscape.getProbability(a) == 1;

    lemon::FilterArcs subgraph(graph, arc_filter);
    ComponentMap componentsNodeMap(graph);
    const int nb_components = lemon::stronglyConnectedComponents(subgraph, componentsNodeMap);

    std::vector<std::vector<Graph::Node>> components(nb_components);
    for(Graph::NodeIt u(graph); u!=lemon::INVALID; ++u)
        components[componentsNodeMap[u]].emplace_back(u);

    for(int component_id=0; component_id<nb_components; ++component_id) {
        const auto & component = components[component_id];
        if(component.size() == 1) continue;
        Graph::Node patch = landscape.addNode(0, Point(0,0));
        for(const auto & u : component) {
            landscape.getQualityRef(patch) += landscape.getQuality(u);
            landscape.getCoordsRef(patch) += landscape.getQuality(u) * landscape.getCoords(u);
            for(Graph::InArcIt a(graph, u), next_a = a; a != lemon::INVALID; a = next_a) {
                ++next_a;
                if(componentsNodeMap[graph.source(a)] == component_id) continue;
                landscape.changeTarget(a, patch);
            }
            for(Graph::OutArcIt b(graph, u), next_b = b; b != lemon::INVALID; b = next_b) {
                ++next_b;
                if(componentsNodeMap[graph.target(b)] == component_id) continue;
                landscape.changeSource(b, patch);
            }
            landscape.removeNode(u);
        }
        landscape.getCoordsRef(patch) /= landscape.getQuality(patch);
    }

}


#endif //TRIVIAL_REFORMULATION_HPP