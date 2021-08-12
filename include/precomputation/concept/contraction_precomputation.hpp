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
    StaticLandscape::Node t;

    ContractionResult(const MutableLandscape & contracted_landscape,
                      const RestorationPlan<MutableLandscape> & contracted_plan,
                      MutableLandscape::Node contracted_t)
        : plan(landscape) {
        const MutableLandscape::Graph & contracted_graph =
            contracted_landscape.getNetwork();
        MutableLandscape::Graph::NodeMap<StaticLandscape::Node> final_nodesRef(
            contracted_graph);
        MutableLandscape::Graph::ArcMap<StaticLandscape::Arc> final_arcsRef(
            contracted_graph);
        landscape.build(contracted_landscape, final_nodesRef, final_arcsRef);
        Helper::copyPlan(plan, contracted_plan, final_nodesRef, final_arcsRef);
        t = final_nodesRef[contracted_t];
    }
    ~ContractionResult() {}
};

class ContractionPrecomputation {
public:
    void remove_unconnected_nodes(MutableLandscape & landscape,
                                  MutableLandscape::Node t) const;
    void contract_arc(MutableLandscape & contracted_landscape,
                      RestorationPlan<MutableLandscape> & plan,
                      MutableLandscape::Arc a) const;
    void contract_restorable_arc(MutableLandscape & landscape,
                                 RestorationPlan<MutableLandscape> & plan,
                                 MutableLandscape::Arc a) const;

    virtual std::unique_ptr<
        MutableLandscape::Graph::NodeMap<std::shared_ptr<ContractionResult>>>
    precompute(const MutableLandscape & landscape,
               const RestorationPlan<MutableLandscape> & plan) const = 0;
};

#endif  // CONTRACTION_PRECOMPUTATION_HPP