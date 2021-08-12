#include "precomputation/concept/contraction_precomputation.hpp"

#include <lemon/dfs.h>

/**
 * Erase the nodes from wich there is no path to t
 * 
 * @time \f$O(|A|)\f$
 * @space \f$O(|A|)\f$
 */
void ContractionPrecomputation::remove_unconnected_nodes(MutableLandscape & landscape, Graph_t::Node t) const {    
    using Reversed = lemon::ReverseDigraph<const Graph_t>;
    const Graph_t & graph = landscape.getNetwork();
    Reversed rg(graph);
    lemon::Dfs<Reversed> dfs(rg);
    dfs.run(t);
    for(Graph_t::NodeIt u(graph), next_u=u; u!=lemon::INVALID; u=next_u) {
        ++next_u;
        if(dfs.reached(u)) continue;
        landscape.removeNode(u);
    }
}

/**
 * Contracts specified uv arc preserving graph ids
 * 
 * @time \f$O(deg(u))\f$
 * @space \f$O(deg(u))\f$
 */
void ContractionPrecomputation::contract_arc(MutableLandscape & landscape, RestorationPlan<MutableLandscape>& plan, Graph_t::Arc a) const {    
    const Graph_t & graph = landscape.getNetwork();
    assert(graph.valid(a));
    Graph_t::Node u = graph.source(a);
    Graph_t::Node v = graph.target(a);
    const double a_probability = landscape.getProbability(a);
    for(Graph_t::InArcIt b(graph, u), next_b = b; b != lemon::INVALID; b = next_b) {
        ++next_b;
        landscape.changeTarget(b, v);
        landscape.getProbabilityRef(b) *= a_probability;
        for(auto & e : plan[b])
            e.restored_probability *= a_probability;
    }
    landscape.getQualityRef(v) += a_probability * landscape.getQuality(u);
    for(auto & e : plan[u])
        plan.addNode(e.option, v, a_probability * e.quality_gain);
    landscape.removeNode(u);
}

static bool restorable_arc_is_contractable(const MutableLandscape & landscape, const RestorationPlan<MutableLandscape>& plan, Graph_t::Arc a) {
    const Graph_t & graph = landscape.getNetwork();
    if(plan[a].size() > 1) return false;
    RestorationPlan<MutableLandscape>::Option a_option = plan[a].begin()->option;
    Graph_t::Node u = graph.source(a);

    for(Graph_t::InArcIt b(graph, u); b!=lemon::INVALID; ++b) {
        if(plan[b].size() != 1) return false;
        if(plan[b].begin()->option != a_option)
            return false;
    }
    if(!plan.contains(u)) return true;
    return plan[u].begin()->option == a_option || plan[u].size() == 1;
}

/**
 * Contracts specified uv arc preserving graph ids
 * 
 * @time \f$O(deg(u))\f$
 * @space \f$O(deg(u))\f$
 */
void ContractionPrecomputation::contract_restorable_arc(MutableLandscape & landscape,
                                       RestorationPlan<MutableLandscape> & plan, 
                                       Graph_t::Arc a) const {    
    const Graph_t & graph = landscape.getNetwork();
    assert(graph.valid(a));
    assert(plan.contains(a));
    static_cast<void>(plan);
    // bool contractable = restorable_arc_is_contractable(landscape, plan, a);

    Graph_t::Node u = graph.source(a);
    // if(!contractable) {
        for(Graph_t::OutArcIt b(graph, u), next_b=b; b!=lemon::INVALID; b=next_b) {
            ++next_b;
            if(a == b) continue;
            landscape.removeArc(b);
        }
    //     return;
    // }

    // Graph_t::Node v = graph.target(a);
    // const double a_probability = landscape.getProbability(a);
    // RestorationPlan<MutableLandscape>::Option a_option = plan[a].begin()->option;
    // const double a_restored_prob = plan[a].begin()->restored_probability;
    // for(Graph_t::InArcIt b(graph, u), next_b=b; b!=lemon::INVALID; b=next_b) {
    //     ++next_b;
    //     landscape.changeTarget(b, v);
    //     landscape.getProbabilityRef(b) *= a_probability;
    //     for(auto & e : plan[b])
    //         e.restored_probability *= a_restored_prob;
    // }

    // landscape.getQualityRef(v) += a_probability * landscape.getQuality(u);
    // plan.addNode(a_option, v, (a_restored_prob-a_probability) * landscape.getQuality(u));
    // for(auto & e : plan[u])
    //     plan.addNode(e.option, v, a_restored_prob * e.quality_gain);
    // landscape.removeNode(u);
    // landscape.getQualityRef(u) = 0;
}