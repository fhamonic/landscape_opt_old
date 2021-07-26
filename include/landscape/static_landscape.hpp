/**
 * @file static_landscape.hpp
 * @author François Hamonic (francois.hamonic@gmail.com)
 * @brief StaticLandscape class declaration
 * @version 0.1
 * @date 2020-05-08
 */

#ifndef STATIC_LANDSCAPE_HPP
#define STATIC_LANDSCAPE_HPP

#include "lemon/static_graph.h"
#include "lemon/adaptors.h"
#include "lemon/maps.h"

#include "Eigen/Dense"

#include "landscape/concept/abstract_landscape.hpp"

typedef lemon::StaticDigraph StaticGraph_t;

/**
 * @brief Class that represent a non-editable landscape.
 * 
 * This class represent a non-editable landscape which has faster iterators and is more memory efficient than \ref MutableLandscape.
 */
class StaticLandscape : public concepts::AbstractLandscape<StaticGraph_t> {
    private:
        Graph network;
        QualityMap qualityMap;
        CoordsMap coordsMap;
        ProbabilityMap probabilityMap;

    public:
        StaticLandscape();
        ~StaticLandscape();

        /**
         * @brief Makes the current landscape a copy of the one passed in parameter.
         * 
         * @param orig_landscape : the landscape to copy
         * @param nodesRef The node references will be copied into this map.
         * @param arcsRef The arc references will be copied into this map.
         */
        template <typename GR, typename QM, typename PM, typename CM>
        void build(const concepts::AbstractLandscape<GR, QM, PM, CM> & orig_landscape, 
                typename GR::template NodeMap<Node> & nodesRef,
                typename GR::template ArcMap<Arc> & arcsRef) {
            const GR & orig_graph = orig_landscape.getNetwork();
            network.build(orig_graph, nodesRef, arcsRef);
            for(typename GR::NodeIt orig_u(orig_graph); orig_u != lemon::INVALID; ++orig_u) {
                const Node u = nodesRef[orig_u];
                qualityMap[u] = orig_landscape.getQuality(orig_u);
                coordsMap[u] = orig_landscape.getCoords(orig_u);
            }
            for(typename GR::ArcIt orig_a(orig_graph); orig_a != lemon::INVALID; ++orig_a) {
                const Arc a = arcsRef[orig_a];
                probabilityMap[a] = orig_landscape.getProbability(orig_a);
            }
        }
        
        const Graph & getNetwork() const;
        const QualityMap & getQualityMap() const;
        const CoordsMap & getCoordsMap() const;
        const ProbabilityMap & getProbabilityMap() const;

        const double & getQuality(Node u) const;
        const Point & getCoords(Node u) const;
        const double & getProbability(Arc a) const;
};

#endif //STATIC_LANDSCAPE_HPP