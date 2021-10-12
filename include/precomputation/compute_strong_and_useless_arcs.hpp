#ifndef COMPUTE_STRONG_AND_USELESS_ARCS_HPP
#define COMPUTE_STRONG_AND_USELESS_ARCS_HPP

#include <algorithm>
#include <atomic>
#include <execution>
#include <memory>
#include <thread>

#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>

#include "algorithms/identify_strong_arcs.h"

#include "helper.hpp"

template <typename Graph, typename LM,
          typename TR = lemon::IdentifyDefaultTraits<Graph, LM>>
auto compute_strong_and_useless_arcs(const Graph & graph,
                                     const LM & lower_lengths,
                                     const LM & upper_lengths) {
    using Node = typename Graph::Node;
    using NodeIt = typename Graph::NodeIt;
    using NodeMap = typename Graph::NodeMap;
    using Arc = typename Graph::Arc;
    using ArcIt = typename Graph::ArcIt;

    std::vector<Arc> arcs;
    for(ArcIt b(graph); b != lemon::INVALID; ++b) arcs.push_back(b);
    std::atomic<std::size_t> cpt_arc = 0;

    NodeMap<bool> node_filter(graph, false);
    for(Node u : target_nodes) node_filter[u] = true;

    auto contractables_arcs =
        std::make_unique<NodeMap<tbb::concurrent_vector<Arc>>>(graph);
    auto deletables_arcs =
        std::make_unique<NodeMap<tbb::concurrent_vector<Arc>>>(graph);

    std::vector<std::thread> threads;
    for(std::size_t i = 0; i < /*std::thread::hardware_concurrency()*/ 1; ++i) {
        threads.emplace_back([&](void) {
            std::vector<Node> strong_nodes;
            std::vector<Node> useless_nodes;
            lemon::IdentifyStrong<Graph, LM, TR> identifyStrong(
                graph, upper_lengths, lower_lengths);
            lemon::IdentifyUseless<Graph, LM, TR> identifyUseless(
                graph, upper_lengths, lower_lengths);
            identifyStrong.labeledNodesList(strong_nodes);
            identifyUseless.labeledNodesList(useless_nodes);

            for(std::size_t local_cpt{};
                (local_cpt = cpt_arc.fetch_add(1, std::memory_order_relaxed)) <
                arcs.size();) {
                Arc a = arcs[local_cpt];
                identifyStrong.run(a);
                identifyUseless.run(a);
                for(Node u : strong_nodes) {
                    if(!node_filter[u]) continue;
                    contractables_arcs[u].push_back(a);
                }
                for(Node u : useless_nodes) {
                    if(!node_filter[u]) continue;
                    deletables_arcs[u].push_back(a);
                }
                strong_nodes.clear();
                useless_nodes.clear();
            }
        });
    }
    for(auto & thread : threads) thread.join();

    return std::make_pair(contractables_arcs, deletables_arcs);
}


template <typename LS>
std::tuple<LS, RestorationPlan<LS>, typename LS::Node> contract_graph() {
    
}

#endif  // COMPUTE_STRONG_AND_USELESS_ARCS_HPP