#ifndef MY_CONTRACTION_ALGORITHM_HPP
#define MY_CONTRACTION_ALGORITHM_HPP

#include "precomputation/concept/contraction_precomputation.hpp"

#include "lemon/dijkstra.h"

#include "helper.hpp"

class MyContractionAlgorithm : public ContractionPrecomputation {
    public:
        ContractionResult contract(const Landscape & landscape, const RestorationPlan<Landscape> & plan, Graph_t::Node t, const std::vector<Graph_t::Arc> & contractables_arcs, const std::vector<Graph_t::Arc> & orig_deletables_arcs) const;
        Graph_t::NodeMap<ContractionResult> * precompute(const Landscape & landscape, const RestorationPlan<Landscape> & plan) const;
};

#endif //MY_CONTRACTION_ALGORITHM_HPP