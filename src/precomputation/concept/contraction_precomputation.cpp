#include "precomputation/concept/contraction_precomputation.hpp"

/**
 * Contracts specified uv arc preserving graph ids
 * 
 * @time O(deg v)
 * @space O(deg v)
 */
void ContractionPrecomputation::contract_arc(Landscape & landscape, Graph_t::Arc a) const {    
    const Graph_t & graph = landscape.getNetwork();

    assert(graph.valid(a));

    //*
    Graph_t::Node u = graph.source(a);
    Graph_t::Node v = graph.target(a);
    
    const double a_probability = landscape.getProbability(a);

    std::vector<Graph_t::Arc> arcs_to_move;
    for(Graph_t::InArcIt b(graph, u); b != lemon::INVALID; ++b) {
        arcs_to_move.push_back(b);
    }
    for(Graph_t::Arc b : arcs_to_move) {
        landscape.changeTarget(b, v);
        landscape.getProbabilityRef(b) *= a_probability;
    }
    landscape.getQualityRef(v) += a_probability * landscape.getQuality(u);
    landscape.removeNode(u);
}

/**
 * Model quality gains options by appending nodes in the Landscape and saves the arcs created in the vector passed by reference.
 * 
 * @time O(#options)
 * @space O(#options)
 */
void ContractionPrecomputation::model_quality_gains(Landscape & landscape, RestorationPlan & plan, std::vector<std::vector<Graph_t::Arc>> & options_nodes) const {
    const Graph_t & graph = landscape.getNetwork();
    options_nodes = std::vector<std::vector<Graph_t::Arc>>(plan.options().size(), std::vector<Graph_t::Arc>());
    
    for(RestorationPlan::Option * option : plan.options()) {
        const int option_id = option->getId();
        for(Graph_t::Node v : option->nodes()) {
            Graph_t::Node u = landscape.addNode(option->getQualityGain(v), landscape.getCoords(v));
            Graph_t::Arc uv = landscape.addArc(u, v, 1.0);
            options_nodes[option_id].push_back(uv);
        }
        for(Graph_t::Arc uv : options_nodes[option_id]) { // needed because it will invalid the iterator
            Graph_t::Node v = graph.target(uv);
            option->removePatch(v);
        }
    }
}

/**
 * Retrives quality gains options by deleting previously added nodes in the Landscape and reconstruct the corresponding option.
 * 
 * @time O(#options)
 * @space O(#options)
 */
void ContractionPrecomputation::retrive_quality_gains(Landscape & landscape, RestorationPlan & plan, const std::vector<std::vector<Graph_t::Arc>> & options_nodes) const {
    const Graph_t & graph = landscape.getNetwork();
    
    for(RestorationPlan::Option * option : plan.options()) {
        for(Graph_t::Arc uv : options_nodes[option->getId()]) { // these arcs are not in contractable_arcs and v is degree 1 so they can't be deleted by contraction
            Graph_t::Node u = graph.source(uv);
            Graph_t::Node v = graph.target(uv);
            const double quality_gain = landscape.getProbability(uv) * landscape.getQuality(u);
            option->addPatch(v, quality_gain);
            landscape.removeNode(u);
        }
    }
}