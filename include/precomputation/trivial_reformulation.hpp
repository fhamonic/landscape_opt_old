#ifndef TRIVIAL_REFORMULATION_HPP
#define TRIVIAL_REFORMULATION_HPP

#include "landscape/mutable_landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"
#include "solvers/concept/instance.hpp"

#include <lemon/connectivity.h>
#include <range/v3/view/subrange.hpp>
#include <range/v3/view/zip.hpp>


/**
 * Erase every arc whose probability in the best scenario is negligeable,
 * according to **epsilon**.
 * 
 * @time \f$O(|A|)\f$
 * @space \f$O(1)\f$
 */
void remove_zero_probability_arcs(MutableLandscape & landscape, 
                    const RestorationPlan<MutableLandscape> & plan, 
                    const double epsilon=1.0e-13) {
    using Graph = MutableLandscape::Graph; 
    const Graph & graph = landscape.getNetwork();    
    for(Graph::ArcIt a(graph), next_a = a; a != lemon::INVALID; a = next_a) {
        ++next_a;
        double probability = landscape.getProbability(a);
        for(const auto & e : plan[a])
            probability = std::max(probability, e.restored_probability);
        if(probability > epsilon) continue;
        landscape.removeArc(a);
    }
}

/**
 * Erase every node that cannot carry flow because its quality is negligeable,
 * according to **epsilon**, in the best scenario, it cannot be enhanced and 
 * there is no path from a positive quality node to it.
 * 
 * @time \f$O(|V| + |A|)\f$
 * @space \f$O(|V|)\f$
 */
void remove_no_flow_nodes(MutableLandscape & landscape, 
                    const RestorationPlan<MutableLandscape> & plan,
                    const double epsilon=1.0e-13) {
    const Graph_t & graph = landscape.getNetwork();
    lemon::Dfs<Graph_t> dfs(graph);
    dfs.init();
    for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) {
        double quality = landscape.getQuality(u);
        for(const auto & e : plan[u])
            quality += e.quality_gain;
        if(quality <= epsilon) continue;
        dfs.addSource(u);
    }
    dfs.start();
    for(Graph_t::NodeIt u(graph), next_u=u; u!=lemon::INVALID; u=next_u) {
        ++next_u;
        if(dfs.reached(u)) continue;
        landscape.removeNode(u);
    }
}

/**
 * Contract the strongly connected components of the subgraph
 * induced by the arcs of probabilty 1.
 * 
 * @time \f$O(|V| + |A|)\f$
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

/**
 * Aggregate parallel arcs.
 * 
 * @time \f$O((|V| + |A|) * \#options)\f$
 * @space \f$O(|V|)\f$
 */
void aggregate_parallel_arcs(MutableLandscape & landscape, RestorationPlan<MutableLandscape> & plan) {
    using Graph = MutableLandscape::Graph;
    using AdjacencyMap = Graph::NodeMap<Graph::Arc>;
    
    const Graph & graph = landscape.getNetwork();
    AdjacencyMap adjacency(graph, lemon::INVALID);

    for(Graph::NodeIt u(graph); u!=lemon::INVALID; ++u) {
        for(Graph::OutArcIt a(graph,u), next_a=a; a!=lemon::INVALID; a=next_a) {
            ++next_a;
            Graph::Node v = graph.target(a);
            Graph::Arc b = adjacency[v];
            if(b == lemon::INVALID || graph.source(b) != u) {
                adjacency[v] = a;
                continue;
            }
            landscape.getProbabilityRef(b) = std::max(
                                landscape.getProbabilityRef(b),
                                landscape.getProbabilityRef(a));
            for(const auto & e : plan[a])
                plan.addArc(e.option, b, e.restored_probability);     
            
            landscape.removeArc(a);      
        }
    }
}

/**
 * Removes empty options from the **plan**.
 * 
 * @time \f$O((|V| + |A|) * \#options)\f$
 * @space \f$O((|V| + |A|) * \#options)\f$
 */
Instance copy_and_compact_instance(const MutableLandscape & landscape, 
                  const RestorationPlan<MutableLandscape> & plan) {
    Instance instance;
    auto refs = instance.landscape.copy(landscape);
    RestorationPlan<MutableLandscape> & new_plan = instance.plan;

    auto nodeOptions = plan.computeNodeOptionsMap();
    auto arcOptions = plan.computeArcOptionsMap();

    for(int i=0; i<plan.getNbOptions(); ++i) {
        if(nodeOptions[i].empty() && arcOptions[i].empty()) continue;
        RestorationPlan<MutableLandscape>::Option option =
            new_plan.addOption(plan.getCost(i));
        for(const auto & [u, quality_gain] : nodeOptions[i])
            new_plan.addNode(option, (*refs.first)[u], quality_gain);
        for(const auto & [a, restored_probability] : arcOptions[i])
            new_plan.addArc(option, (*refs.second)[a], restored_probability);
    }

    return instance;
}

/**
 * Performs trivial simplifications of the given instance such as :
 * - removing arc whose probability is negligeable
 * - removing nodes that can only carry negligeable flow
 * - contracting the strongly connected components formed by 
 *      arcs of probability 1
 * - aggregating parallels arcs
 * 
 * @time \f$O((|V| + |A|) * \#options)\f$
 * @space \f$O((|V| + |A|) * \#options)\f$
 */
Instance trivial_reformulate(MutableLandscape&& landscape, 
            RestorationPlan<MutableLandscape>&& plan) {
    remove_zero_probability_arcs(landscape, plan);
    remove_no_flow_nodes(landscape, plan);
    contract_patches_components(landscape, plan);
    aggregate_parallel_arcs(landscape, plan);
    return copy_and_compact_instance(landscape, plan);
}
Instance trivial_reformulate(Instance&& instance) {
    return trivial_reformulate(std::move(instance.landscape), 
                               std::move(instance.plan));
}
Instance trivial_reformulate(const MutableLandscape & landscape,
            const RestorationPlan<MutableLandscape> & plan) {
    return trivial_reformulate(copy_and_compact_instance(landscape, plan));
}
Instance trivial_reformulate(const Instance & instance) {
    return trivial_reformulate(instance.landscape, instance.plan);
}


#endif //TRIVIAL_REFORMULATION_HPP