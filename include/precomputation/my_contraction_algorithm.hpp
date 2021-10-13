#ifndef MY_CONTRACTION_ALGORITHM_HPP
#define MY_CONTRACTION_ALGORITHM_HPP

#include <algorithm>
#include <atomic>
#include <execution>
#include <memory>
#include <thread>

#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_vector.h>

#include "algorithms/identify_strong_arcs.h"
#include "precomputation/concept/contraction_precomputation.hpp"

#include "helper.hpp"

class MyContractionAlgorithm : public ContractionPrecomputation {
public:
    template <typename ArcList>
    std::shared_ptr<ContractionResult> contract(
        const MutableLandscape & landscape,
        const RestorationPlan<MutableLandscape> & plan,
        MutableLandscape::Node orig_t, ArcList & orig_contractables_arcs,
        ArcList & orig_deletables_arcs) const {
        using Graph = MutableLandscape::Graph;

        MutableLandscape contracted_landscape;
        auto refs = contracted_landscape.copy(landscape);
        RestorationPlan<MutableLandscape> contracted_plan(contracted_landscape);
        Helper::copyPlan(contracted_plan, plan, *refs.first, *refs.second);
        Graph::Node contracted_t = (*refs.first)[orig_t];

        remove_unconnected_nodes(contracted_landscape, contracted_t);
        const Graph & contracted_graph = contracted_landscape.getNetwork();

        for(Graph::Arc orig_a : orig_deletables_arcs) {
            Graph::Arc a = (*refs.second)[orig_a];
            if(!contracted_graph.valid(a)) continue;
            contracted_landscape.removeArc(a);
        }

        for(Graph::Arc orig_a : orig_contractables_arcs) {
            Graph::Arc a = (*refs.second)[orig_a];
            if(!contracted_graph.valid(a)) continue;
            if(contracted_plan.contains(a)) {
                contract_restorable_arc(contracted_landscape, contracted_plan,
                                        a);
                continue;
            }
            contract_arc(contracted_landscape, contracted_plan, a);
        }

        remove_no_flow_nodes(contracted_landscape, contracted_plan);

        return std::make_shared<ContractionResult>(
            contracted_landscape, contracted_plan, contracted_t);
    }

    std::unique_ptr<
        MutableLandscape::Graph::NodeMap<std::shared_ptr<ContractionResult>>>
    precompute(const MutableLandscape & landscape,
               const RestorationPlan<MutableLandscape> & plan,
               const std::vector<MutableLandscape::Node> & target_nodes) const;
    std::unique_ptr<
        MutableLandscape::Graph::NodeMap<std::shared_ptr<ContractionResult>>>
    precompute(const MutableLandscape & landscape,
               const RestorationPlan<MutableLandscape> & plan) const;
};

#endif  // MY_CONTRACTION_ALGORITHM_HPP