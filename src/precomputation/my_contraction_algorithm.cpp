#include "precomputation/my_contraction_algorithm.hpp"

#include <algorithm>
#include <execution>

#include "algorithms/identify_strong_arcs.h"

class NodeDist {
    public:
        double d;
        bool b;

        NodeDist() : d(std::numeric_limits<double>::max()), b(false) {}
        NodeDist(double d) : d(d), b(false) {}
        NodeDist(double d, bool b) : d(d), b(b) {}
       
        bool operator<(const NodeDist o) const { return d < o.d || (d == o.d && b && !o.b); }
        bool operator==(const NodeDist o) const { return d == o.d && b == o.b; }

        bool operator<=(const NodeDist o) const { return (*this < o) || (*this == o); }

        bool operator>=(const NodeDist o) const { return !(*this < o); }
        bool operator>(const NodeDist o) const { return !(*this <= o); }
        bool operator!=(const NodeDist o) const { return !(*this == o); }

        NodeDist operator+(const NodeDist o) const { return NodeDist(d + o.d, b || o.b); }

        NodeDist & operator+=(const NodeDist o) {
            d += o.d;
            b = b || o.b;
            return *this;
        }
        NodeDist & operator=(const NodeDist o) {
            d = o.d;
            b = o.b;
            return *this;
        }
};

// void MyContractionAlgorithm::getStrongs(const Graph_t & graph, const Graph_t::ArcMap<double> & l_min, const Graph_t::ArcMap<double> & l_max, Graph_t::Arc uv, std::vector<Graph_t::Node> & strong_nodes) const {
//     const Graph_t::Node u = graph.source(uv);
    
//     Graph_t::ArcMap<NodeDist> lengths(graph);
//     for(Graph_t::ArcIt b(graph); b != lemon::INVALID; ++b)
//         lengths[b] = NodeDist(l_min[b], false);
//     lengths[uv] = NodeDist(l_max[uv], true);

//     lemon::Dijkstra<Graph_t, Graph_t::ArcMap<NodeDist>> dijkstra(graph, lengths);
//     dijkstra.init();
//     dijkstra.addSource(u, NodeDist(0, false));

//     while (!dijkstra.emptyQueue()) {
//         Graph_t::Node x = dijkstra.nextNode();
//         NodeDist dx =  dijkstra.currentDist(x);
//         const bool strong = dx.b;
//         if(strong) {
//             for(Graph_t::OutArcIt b(graph, x); b != lemon::INVALID; ++b)
//                 lengths[b] = NodeDist(l_max[b], false);
//             strong_nodes.push_back(x);
//         }
//         dijkstra.processNextNode();
//     }
// }

// void MyContractionAlgorithm::getNonWeaks(const Graph_t & graph, const Graph_t::ArcMap<double> & l_min, const Graph_t::ArcMap<double> & l_max, Graph_t::Arc uv, std::vector<Graph_t::Node> & non_weak_nodes) const {
//     const Graph_t::Node u = graph.source(uv);
    
//     Graph_t::ArcMap<NodeDist> lengths(graph);
//     for(Graph_t::ArcIt b(graph); b != lemon::INVALID; ++b)
//         lengths[b] = NodeDist(l_max[b], false);
//     lengths[uv] = NodeDist(l_min[uv], true);

//     lemon::Dijkstra<Graph_t, Graph_t::ArcMap<NodeDist>> dijkstra(graph, lengths);
//     dijkstra.init();
//     dijkstra.addSource(u, NodeDist(0, false));

//     while (!dijkstra.emptyQueue()) {
//         Graph_t::Node x = dijkstra.nextNode();
//         NodeDist dx = dijkstra.currentDist(x);
//         const bool weak = dx.b;
//         if(weak) {
//             for(Graph_t::OutArcIt b(graph, x); b != lemon::INVALID; ++b)
//                 lengths[b] = NodeDist(l_min[b], false);
//         } else {
//             non_weak_nodes.push_back(x);
//         }
//         dijkstra.processNextNode();
//     }
// }



ContractionResult MyContractionAlgorithm::contract(const Landscape & landscape, const RestorationPlan & plan, Graph_t::Node orig_t, const std::vector<Graph_t::Arc> & orig_contractables_arcs, const std::vector<Graph_t::Arc> & orig_deletables_arcs) const {
    Landscape * contracted_landscape = new Landscape();
    std::pair<Graph_t::NodeMap<Graph_t::Node>*, Graph_t::ArcMap<Graph_t::Arc>*> refs = contracted_landscape->copy(landscape);
    RestorationPlan * contracted_plan = new RestorationPlan(*contracted_landscape);
    Helper::copyPlan(*contracted_plan, plan, *refs.first, *refs.second);
    Graph_t::Node contracted_t = (*refs.first)[orig_t];

    const Graph_t & graph = contracted_landscape->getNetwork();
    erase_non_connected(*contracted_landscape, contracted_t);

    for(Graph_t::Arc orig_a : orig_deletables_arcs) {   
        Graph_t::Arc a = (*refs.second)[orig_a];  
        if(!graph.valid(a)) continue;
        contracted_landscape->removeArc(a);
    }
    for(Graph_t::Arc orig_a : orig_contractables_arcs) {
        Graph_t::Arc a = (*refs.second)[orig_a];
        if(!graph.valid(a)) continue;
        if(contracted_plan->contains(a)) continue;
        contract_arc(*contracted_landscape, *contracted_plan, a);
    }
    
    contracted_plan->eraseInvalidElements();

    // ///////// reduce memory usage -> TODO StaticLandscape class
    Landscape * final_landscape = new Landscape();
    std::pair<Graph_t::NodeMap<Graph_t::Node>*, Graph_t::ArcMap<Graph_t::Arc>*> final_refs = final_landscape->copy(*contracted_landscape);
    RestorationPlan * final_plan = new RestorationPlan(*final_landscape);
    Helper::copyPlan(*final_plan, *contracted_plan, *final_refs.first, *final_refs.second);
    Graph_t::Node final_t = (*final_refs.first)[contracted_t];

    delete contracted_landscape;
    delete contracted_plan;
    delete refs.first;
    delete refs.second;
    delete final_refs.first;
    delete final_refs.second;

    return ContractionResult(final_landscape, final_plan, final_t);
}


Graph_t::NodeMap<ContractionResult> * MyContractionAlgorithm::precompute(const Landscape & landscape, const RestorationPlan & plan) const {
    const Graph_t & graph = landscape.getNetwork();
    Graph_t::NodeMap<ContractionResult> * results = new Graph_t::NodeMap<ContractionResult>(graph);

    //const Graph_t::ArcMap<double> & p_min = landscape.getProbabilityMap(); // since log is needed
    Graph_t::ArcMap<double> p_min(graph);
    Graph_t::ArcMap<double> p_max(graph);
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
        p_max[a] = p_min[a] = landscape.getProbability(a);
        for(auto const& [i, restored_probability] : plan.getOptions(a))
            p_max[a] = std::max(p_max[a], restored_probability);
    }

    // TODO use graph iterator
    std::vector<Graph_t::Node> nodes;
    for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) nodes.push_back(u);
    std::vector<Graph_t::Arc> arcs;
    for(Graph_t::ArcIt b(graph); b != lemon::INVALID; ++b) arcs.push_back(b);

    Graph_t::ArcMap<std::vector<Graph_t::Node>> strong_nodes(graph);
    Graph_t::ArcMap<std::vector<Graph_t::Node>> non_weak_nodes(graph);

    //*
    for(Graph_t::ArcIt b(graph); b != lemon::INVALID; ++b) {
        p_max[b] = -std::log(p_max[b]);
        p_min[b] = -std::log(p_min[b]);
    }//*/
    std::for_each(std::execution::par, arcs.begin(), arcs.end(), [&] (Graph_t::Arc a) {
        //*
        lemon::IdentifyStrong<Graph_t, Graph_t::ArcMap<double>> identifyStrong(graph, p_min, p_max);
        lemon::IdentifyUseless<Graph_t, Graph_t::ArcMap<double>> identifyUseless(graph, p_min, p_max);
        identifyStrong.labeledNodesList(strong_nodes[a]).run(a);
        identifyUseless.labeledNodesList(non_weak_nodes[a]).run(a);
        /*/
        getStrongs(graph, p_max, p_min, a, strong_nodes[a]);
        getNonWeaks(graph, p_max, p_min, a, non_weak_nodes[a]);
        //*/
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

    std::for_each(std::execution::par, nodes.begin(), nodes.end(), [&] (Graph_t::Node u) {
        (*results)[u] = contract(landscape, plan, u, contractables_arcs[u], deletables_arcs[u]);
    });
    return results;
}