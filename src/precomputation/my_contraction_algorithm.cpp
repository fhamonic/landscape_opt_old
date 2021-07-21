#include "precomputation/my_contraction_algorithm.hpp"

#include <algorithm>
#include <execution>

#include "algorithms/identify_strong_arcs.h"

ContractionResult MyContractionAlgorithm::contract(const Landscape & landscape, const RestorationPlan<Landscape> & plan, Graph_t::Node orig_t, const std::vector<Graph_t::Arc> & orig_contractables_arcs, const std::vector<Graph_t::Arc> & orig_deletables_arcs) const {
    Landscape contracted_landscape;
    std::pair<Graph_t::NodeMap<Graph_t::Node>*, Graph_t::ArcMap<Graph_t::Arc>*> refs = contracted_landscape.copy(landscape);
    RestorationPlan<Landscape>contracted_plan(contracted_landscape);
    Helper::copyPlan(contracted_plan, plan, *refs.first, *refs.second);
    Graph_t::Node contracted_t = (*refs.first)[orig_t];

    const Graph_t & contracted_graph = contracted_landscape.getNetwork();
    erase_no_connected_nodes(contracted_landscape, contracted_t);

    for(Graph_t::Arc orig_a : orig_deletables_arcs) {   
        Graph_t::Arc a = (*refs.second)[orig_a];  
        if(!contracted_graph.valid(a)) continue;
        contracted_landscape.removeArc(a);
    }

    for(Graph_t::Arc orig_a : orig_contractables_arcs) {
        Graph_t::Arc a = (*refs.second)[orig_a];
        if(!contracted_graph.valid(a)) continue;
        if(contracted_plan.contains(a)) continue;
        contract_arc(contracted_landscape, contracted_plan, a);
    }

    erase_no_flow_nodes(contracted_landscape, contracted_plan);

    // ///////// reduce memory usage -> TODO StaticLandscape class
    StaticLandscape * final_landscape = new StaticLandscape();
    Graph_t::NodeMap<StaticGraph_t::Node> final_nodesRef(contracted_graph);
    Graph_t::ArcMap<StaticGraph_t::Arc> final_arcsRef(contracted_graph);
    final_landscape->build(contracted_landscape, final_nodesRef, final_arcsRef);
    RestorationPlan<StaticLandscape> * final_plan = new RestorationPlan(*final_landscape);
    Helper::copyPlan(*final_plan, contracted_plan, final_nodesRef, final_arcsRef);
    StaticGraph_t::Node final_t = final_nodesRef[contracted_t];

    delete refs.first;
    delete refs.second;

    return ContractionResult(final_landscape, final_plan, final_t);
}


Graph_t::NodeMap<ContractionResult> * MyContractionAlgorithm::precompute(const Landscape & landscape, const RestorationPlan<Landscape> & plan) const {
    const Graph_t & graph = landscape.getNetwork();
    Graph_t::NodeMap<ContractionResult> * results = new Graph_t::NodeMap<ContractionResult>(graph);

    const Graph_t::ArcMap<double> & p_min = landscape.getProbabilityMap();
    Graph_t::ArcMap<double> p_max(graph);
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
        p_max[a] = p_min[a];
        for(const auto & e : plan[a])
            p_max[a] = std::max(p_max[a], e.restored_probability);
    }

    // TODO use graph iterator
    std::vector<Graph_t::Node> nodes;
    for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) nodes.push_back(u);
    std::vector<Graph_t::Arc> arcs;
    for(Graph_t::ArcIt b(graph); b != lemon::INVALID; ++b) arcs.push_back(b);

    Graph_t::ArcMap<std::vector<Graph_t::Node>> strong_nodes(graph);
    Graph_t::ArcMap<std::vector<Graph_t::Node>> non_weak_nodes(graph);

    std::for_each(std::execution::par_unseq, arcs.begin(), arcs.end(), [&] (Graph_t::Arc a) {        
        lemon::MultiplicativeIdentifyStrong<Graph_t, Graph_t::ArcMap<double>> identifyStrong(graph, p_min, p_max);
        lemon::MultiplicativeIdentifyUseless<Graph_t, Graph_t::ArcMap<double>> identifyUseless(graph, p_min, p_max);
        identifyStrong.labeledNodesList(strong_nodes[a]).run(a);
        identifyUseless.labeledNodesList(non_weak_nodes[a]).run(a);
    });

    // transpose
    Graph_t::NodeMap<std::vector<Graph_t::Arc>> contractables_arcs(graph);
    Graph_t::NodeMap<std::vector<Graph_t::Arc>> deletables_arcs(graph);
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
        for(Graph_t::Node u : strong_nodes[a])
            contractables_arcs[u].push_back(a);
        for(Graph_t::Node u : non_weak_nodes[a])
            deletables_arcs[u].push_back(a);
    }

    std::for_each(std::execution::par_unseq, nodes.begin(), nodes.end(), [&] (Graph_t::Node u) {
        (*results)[u] = contract(landscape, plan, u, contractables_arcs[u], deletables_arcs[u]);
    });
    return results;
}