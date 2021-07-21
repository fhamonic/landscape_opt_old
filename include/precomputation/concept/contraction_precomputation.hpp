#ifndef CONTRACTION_PRECOMPUTATION_HPP
#define CONTRACTION_PRECOMPUTATION_HPP

#include <memory>

#include "landscape/landscape.hpp"
#include "landscape/static_landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"

#include "indices/eca.hpp"

#include "helper.hpp"

class ContractionResult {
    public:
        std::shared_ptr<StaticLandscape> landscape;
        std::shared_ptr<RestorationPlan<StaticLandscape>> plan;
        StaticGraph_t::Node t;

        ContractionResult() {}
        ContractionResult(StaticLandscape * l, RestorationPlan<StaticLandscape> * p, StaticGraph_t::Node t) : landscape(l), plan(p), t(t) {}
        ~ContractionResult() {}
};

class ContractionPrecomputation {
    public:
        void erase_no_connected_nodes(Landscape & landscape, Graph_t::Node t) const;
        void erase_no_flow_nodes(Landscape & landscape, const RestorationPlan<Landscape> & plan) const;
        void contract_arc(Landscape & contracted_landscape, RestorationPlan<Landscape> & plan, Graph_t::Arc a) const;
        
        virtual Graph_t::NodeMap<ContractionResult> * precompute(const Landscape & landscape, const RestorationPlan<Landscape> & plan) const=0;
};

#endif //CONTRACTION_PRECOMPUTATION_HPP