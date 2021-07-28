#ifndef CONTRACTION_PRECOMPUTATION_HPP
#define CONTRACTION_PRECOMPUTATION_HPP

#include <memory>

#include "landscape/mutable_landscape.hpp"
#include "landscape/static_landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"

#include "indices/eca.hpp"

#include "precomputation/trivial_reformulation.hpp"

#include "helper.hpp"

class ContractionResult {
    public:
        StaticLandscape landscape;
        RestorationPlan<StaticLandscape> plan;
        StaticGraph_t::Node t;

        ContractionResult(const MutableLandscape & contracted_landscape,
                    const RestorationPlan<MutableLandscape> & contracted_plan, 
                    Graph_t::Node contracted_t) : plan(landscape) {
            const Graph_t & contracted_graph = contracted_landscape.getNetwork();
            Graph_t::NodeMap<StaticGraph_t::Node> final_nodesRef(contracted_graph);
            Graph_t::ArcMap<StaticGraph_t::Arc> final_arcsRef(contracted_graph);
            landscape.build(contracted_landscape, final_nodesRef, final_arcsRef);
            Helper::copyPlan(plan, contracted_plan, final_nodesRef, final_arcsRef);
            t = final_nodesRef[contracted_t];
        }
        ~ContractionResult() {}
};

class ContractionPrecomputation {
    public:
        void remove_unconnected_nodes(MutableLandscape & landscape, Graph_t::Node t) const;
        void contract_arc(MutableLandscape & contracted_landscape, RestorationPlan<MutableLandscape> & plan, Graph_t::Arc a) const;
        
        virtual std::unique_ptr<Graph_t::NodeMap<std::shared_ptr<ContractionResult>>>
            precompute(const MutableLandscape & landscape, const RestorationPlan<MutableLandscape> & plan) const=0;
};

#endif //CONTRACTION_PRECOMPUTATION_HPP