#ifndef MY_CONTRACTION_ALGORITHM_HPP
#define MY_CONTRACTION_ALGORITHM_HPP

#include "precomputation/concept/contraction_precomputation.hpp"

#include "lemon/dijkstra.h"

#include "helper.hpp"

class MyContractionAlgorithm : public ContractionPrecomputation {
    public:
        void getStrongs(const Graph_t & graph, const Graph_t::ArcMap<double> & l_min, const Graph_t::ArcMap<double> & l_max, Graph_t::Arc uv, std::vector<Graph_t::Node> & strong_nodes) const;
        void getNonWeaks(const Graph_t & graph, const Graph_t::ArcMap<double> & l_min, const Graph_t::ArcMap<double> & l_max, Graph_t::Arc uv, std::vector<Graph_t::Node> & non_weak_nodes) const;
        
        ContractionResult contract(const Landscape & landscape, const RestorationPlan & plan, Graph_t::Node t, const std::vector<Graph_t::Arc> & contractables_arcs, const std::vector<Graph_t::Arc> & orig_deletables_arcs) const;
        Graph_t::NodeMap<ContractionResult> * precompute(const Landscape & landscape, const RestorationPlan & plan) const;
};

#endif //MY_CONTRACTION_ALGORITHM_HPP