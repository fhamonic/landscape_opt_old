#ifndef TRIVIAL_REFORMULATION_HPP
#define TRIVIAL_REFORMULATION_HPP

#include "landscape/mutable_landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"

#include <lemon/connectivity.h>
#include <range/v3/view/subrange.hpp>
#include <range/v3/view/zip.hpp>

RestorationPlan<MutableLandscape> remove_useless_options(const MutableLandscape & landscape, RestorationPlan<MutableLandscape> & plan, const double epsilon=1.0e-13) {
    RestorationPlan<MutableLandscape> new_plan(landscape);

    auto nodeOptions = plan.computeNodeOptionsMap();
    auto arcOptions = plan.computeArcOptionsMap();

    nodeOptions.erase(
        std::remove_if(nodeOptions.begin(), nodeOptions.end(), [&](const auto & e) {
            return e.second <= epsilon;
        }),
        nodeOptions.end());
    arcOptions.erase(
        std::remove_if(arcOptions.begin(), arcOptions.end(), [&](const auto & e) {
            return e.second <= epsilon;
        }),
        arcOptions.end());

    int cpt_options = 0;
    for(const auto & [nodes, arcs] : ranges::views::zip(nodeOptions, arcOptions)) {
        if(nodes.empty() && arcs.empty()) continue;
        for(const auto & [u, quality_gain] : nodes)
            new_plan.addNode(cpt_options, u, quality_gain);
        for(const auto & [a, restored_probability] : arcs)
            new_plan.addArc(cpt_options, a, restored_probability);
        ++cpt_options;
    }
}

/**
 * Erase every arc whose probability is negligeable.
 * 
 * @time \f$O(|A|)\f$
 * @space \f$O(|A|)\f$
 */
void remove_zero_probability_arcs(MutableLandscape & landscape, const RestorationPlan<MutableLandscape> & plan, const double epsilon=1.0e-13) {
    using Graph = MutableLandscape::Graph; 
    const Graph & graph = landscape.getNetwork();    
    for(Graph::ArcIt a(graph), next_a = a; a != lemon::INVALID; a = next_a) {
        ++next_a;
        if(landscape.getProbability(a) > epsilon || plan.contains(a)) continue;
        landscape.removeArc(a);
    }
}

/**
 * Erase every node that cannot carry flow because its quality is null,
 * it cannot be enhanced and there is no path from a positive quality node to it.
 * 
 * @time \f$O(|A|)\f$
 * @space \f$O(|A|)\f$
 */
void remove_no_flow_nodes(MutableLandscape & landscape, const RestorationPlan<MutableLandscape> & plan) {
    const Graph_t & graph = landscape.getNetwork();
    lemon::Dfs<Graph_t> dfs(graph);
    dfs.init();
    for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) {
        if(landscape.getQuality(u) == 0 && !plan.contains(u)) continue;
        dfs.addSource(u);
    }
    dfs.start();
    for(Graph_t::NodeIt u(graph), next_u = u; u != lemon::INVALID; u = next_u) {
        ++next_u;
        if(dfs.reached(u)) continue;
        landscape.removeNode(u);
    }
}

/**
 * Contract the strongly connected components of the subgraph
 * induced by the arcs of probabilty 1.
 * 
 * @time \f$O(|A|)\f$
 * @space \f$O(|A|)\f$
 */
void contract_patches_components(MutableLandscape & landscape, RestorationPlan<MutableLandscape> & plan) {
    using Graph = MutableLandscape::Graph;
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
        const Graph::Node patch = *component.begin();
        landscape.getCoordsRef(patch) *= landscape.getQuality(patch);
        for(const auto & u : ranges::subrange(component.begin()+1, 
                                              component.end())) {
            landscape.getQualityRef(patch) += landscape.getQuality(u);
            for(const auto & e : plan[u])
                plan.addNode(e.option, patch, e.quality_gain);
            landscape.getCoordsRef(patch) +=
                landscape.getCoords(u) * landscape.getQuality(u);
            for(Graph::InArcIt a(graph,u), next_a=a; a!=lemon::INVALID; a=next_a) {
                ++next_a;
                if(componentsNodeMap[graph.source(a)] == component_id) continue;
                landscape.changeTarget(a, patch);
            }
            for(Graph::OutArcIt b(graph,u), next_b=b; b!=lemon::INVALID; b=next_b) {
                ++next_b;
                if(componentsNodeMap[graph.target(b)] == component_id) continue;
                landscape.changeSource(b, patch);
            }
            landscape.removeNode(u);
        }
        landscape.getCoordsRef(patch) /= landscape.getQuality(patch);
    }
}

std::pair<MutableLandscape, RestorationPlan<MutableLandscape>> trivial_reformulate(MutableLandscape&& landscape, RestorationPlan<MutableLandscape>&& plan) {

    contract_patches_components(landscape, plan);

}

// std::pair<MutableLandscape, RestorationPlan<MutableLandscape>> trivial_reformulate(const MutableLandscape & original_landscape, const RestorationPlan<MutableLandscape> & original_plan) {



//     // return contract_patches_components(std::move(copy_landscape), std::move(copy_plan));
// }


#endif //TRIVIAL_REFORMULATION_HPP