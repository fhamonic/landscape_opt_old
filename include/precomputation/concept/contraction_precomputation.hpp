#ifndef CONTRACTION_PRECOMPUTATION_HPP
#define CONTRACTION_PRECOMPUTATION_HPP

#include <memory>

#include "landscape/landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"

#include "indices/eca.hpp"

#include "helper.hpp"

class ContractionResult {
    public:
        std::shared_ptr<Landscape> landscape;
        std::shared_ptr<RestorationPlan> plan;
        Graph_t::Node t;

        ContractionResult() {}
        ContractionResult(Landscape * l, RestorationPlan * p, Graph_t::Node t) : landscape(l), plan(p), t(t) {}
        ~ContractionResult() {}
};

class ContractionPrecomputation {
    public:
        void contract_arc(Landscape & contracted_landscape, Graph_t::Arc a) const;
        void model_quality_gains(Landscape & landscape, RestorationPlan & plan, std::vector<std::vector<Graph_t::Arc>> & options_nodes) const;
        void retrive_quality_gains(Landscape & landscape, RestorationPlan & plan, const std::vector<std::vector<Graph_t::Arc>> & options_nodes) const;

        virtual Graph_t::NodeMap<ContractionResult> * precompute(const Landscape & landscape, const RestorationPlan & plan) const=0;
};

#endif //CONTRACTION_PRECOMPUTATION_HPP