#include "precomputation/my_contraction_algorithm.hpp"

std::unique_ptr<Graph_t::NodeMap<std::shared_ptr<ContractionResult>>> MyContractionAlgorithm::precompute(
                const MutableLandscape & landscape, 
                const RestorationPlan<MutableLandscape> & plan, 
                const std::vector<Graph_t::Node> & target_nodes) const {
    using Graph = MutableLandscape::Graph;
    using ProbabilityMap = MutableLandscape::ProbabilityMap;
    
    const Graph & graph = landscape.getNetwork();
    auto results = std::make_unique<Graph::NodeMap<std::shared_ptr<ContractionResult>>>(graph);

    const ProbabilityMap & p_min = landscape.getProbabilityMap();
    ProbabilityMap p_max(graph);
    for(Graph::ArcIt a(graph); a != lemon::INVALID; ++a) {
        p_max[a] = p_min[a];
        for(const auto & e : plan[a])
            p_max[a] = std::max(p_max[a], e.restored_probability);
    }

    tbb::concurrent_queue<Graph::Arc> arcs;
    for(Graph::ArcIt b(graph); b != lemon::INVALID; ++b)
        arcs.push(b);

    Graph::NodeMap<bool> node_filter(graph, false);
    for(Graph::Node u : target_nodes)
        node_filter[u] = true;

    Graph::NodeMap<tbb::concurrent_vector<Graph::Arc>> contractables_arcs(graph);
    Graph::NodeMap<tbb::concurrent_vector<Graph::Arc>> deletables_arcs(graph);

    std::vector<int> threads(std::thread::hardware_concurrency());
    std::for_each(std::execution::par_unseq, threads.begin(), threads.end(), [&] (int) {
        std::vector<Graph::Node> strong_nodes;
        std::vector<Graph::Node> non_weak_nodes;
        lemon::MultiplicativeIdentifyStrong<Graph, ProbabilityMap> identifyStrong(graph, p_min, p_max);
        lemon::MultiplicativeIdentifyUseless<Graph, ProbabilityMap> identifyUseless(graph, p_min, p_max);
        identifyStrong.labeledNodesList(strong_nodes);
        identifyUseless.labeledNodesList(non_weak_nodes);

        Graph::Arc a;
        while(!arcs.empty()) {
            if(!arcs.try_pop(a))
                continue;

            identifyStrong.run(a);
            identifyUseless.run(a);

            for(Graph::Node u : strong_nodes) {
                if(!node_filter[u]) continue;
                contractables_arcs[u].push_back(a);
            }
            for(Graph::Node u : non_weak_nodes) {
                if(!node_filter[u]) continue;
                deletables_arcs[u].push_back(a);
            }

            strong_nodes.clear();
            non_weak_nodes.clear();
        }
    });

    std::for_each(std::execution::par_unseq,
                target_nodes.begin(), 
                target_nodes.end(), 
                [&] (Graph::Node u) {
        (*results)[u] = contract(landscape, plan, u,
                                contractables_arcs[u],
                                deletables_arcs[u]);
    });
    return results;
}

std::unique_ptr<Graph_t::NodeMap<std::shared_ptr<ContractionResult>>> MyContractionAlgorithm::precompute(
                const MutableLandscape & landscape, 
                const RestorationPlan<MutableLandscape> & plan) const {
    using Graph = MutableLandscape::Graph;
    const Graph & graph = landscape.getNetwork();
    std::vector<Graph::Node> nodes;
    for(Graph::NodeIt u(graph); u != lemon::INVALID; ++u)
        nodes.push_back(u);
    return precompute(landscape, plan, nodes);
}