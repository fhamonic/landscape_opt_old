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

    std::vector<Graph::Arc> arcs;
    for(Graph::ArcIt b(graph); b != lemon::INVALID; ++b)
        arcs.push_back(b);
    std::atomic<std::size_t> cpt_arc = 0;

    Graph::NodeMap<bool> node_filter(graph, false);
    for(Graph::Node u : target_nodes)
        node_filter[u] = true;

    Graph::NodeMap<tbb::concurrent_vector<Graph::Arc>> contractables_arcs(graph);
    Graph::NodeMap<tbb::concurrent_vector<Graph::Arc>> deletables_arcs(graph);

    std::vector<std::thread> threads;
    for(std::size_t i=0; i<std::thread::hardware_concurrency(); ++i) {
        threads.emplace_back([&](void) {
            std::vector<Graph::Node> strong_nodes;
            std::vector<Graph::Node> non_weak_nodes;
            lemon::MultiplicativeIdentifyStrong<Graph, ProbabilityMap> identifyStrong(graph, p_min, p_max);
            lemon::MultiplicativeIdentifyUseless<Graph, ProbabilityMap> identifyUseless(graph, p_min, p_max);
            identifyStrong.labeledNodesList(strong_nodes);
            identifyUseless.labeledNodesList(non_weak_nodes);

            for(std::size_t local_cpt{}; (local_cpt=cpt_arc.fetch_add(1, std::memory_order_relaxed)) < arcs.size();) {
                Graph::Arc a = arcs[local_cpt];
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
    }
    for(auto & thread : threads) thread.join();


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