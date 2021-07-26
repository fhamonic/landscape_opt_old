/**
 * @file decored_landscape.hpp
 * @author François Hamonic (francois.hamonic@gmail.com)
 * @brief DecoredLandscape class declaration
 * @version 0.2
 * @date 2021-07-18
 */

#ifndef DECORED_LANDSCAPE_HPP
#define DECORED_LANDSCAPE_HPP

#include "lemon/list_graph.h"
#include "lemon/adaptors.h"
#include "lemon/maps.h"

#include "Eigen/Dense"

#include "landscape/concept/abstract_landscape.hpp"
#include "landscape/mutable_landscape.hpp"

#include "solvers/concept/restoration_plan.hpp"

/**
 * @brief Class that represent an decored landscape.
 * 
 * This class represent a decored landscape.
 * That is a modification of the weights of a reference landscape.
 */
template <typename LS>
class DecoredLandscape
    : public concepts::AbstractLandscape<typename LS::Graph> {
public:
    using Graph = typename LS::Graph;
    using Node = typename LS::Graph::Node;
    using Arc = typename LS::Graph::Arc;
    using QualityMap = typename LS::QualityMap;
    using ProbabilityMap = typename LS::ProbabilityMap;
    using CoordsMap = typename LS::CoordsMap;
private:
    std::reference_wrapper<const Graph> network;
    std::reference_wrapper<const QualityMap> original_qualityMap;
    std::reference_wrapper<const ProbabilityMap> original_probabilityMap;
    std::reference_wrapper<const CoordsMap> coordsMap;
    QualityMap qualityMap;
    ProbabilityMap probabilityMap;

public:
    DecoredLandscape(const Graph & original_network,
        const QualityMap & original_qualityMap,
        const ProbabilityMap & original_probabilityMap,
        const CoordsMap & coordsMap)
        : network(original_network)
        , original_qualityMap(original_qualityMap)
        , original_probabilityMap(original_probabilityMap)
        , coordsMap(coordsMap)
        , qualityMap(network)
        , probabilityMap(network) { reset(); }
    DecoredLandscape(const LS & landscape)
        : network(landscape.getNetwork())
        , original_qualityMap(landscape.getQualityMap())
        , original_probabilityMap(landscape.getProbabilityMap())
        , coordsMap(landscape.getCoordsMap())
        , qualityMap(network)
        , probabilityMap(network) { reset(); }
    DecoredLandscape(const DecoredLandscape<LS> & landscape)
        : network(landscape.network)
        , original_qualityMap(landscape.original_qualityMap)
        , original_probabilityMap(landscape.original_probabilityMap)
        , coordsMap(landscape.coordsMap)
        , qualityMap(network)
        , probabilityMap(network) { 
        for(typename Graph::NodeIt u(network); u != lemon::INVALID; ++u)
            setQuality(u, landscape.getQuality(u));
        for(typename Graph::ArcIt a(network); a != lemon::INVALID; ++a)
            setProbability(a, landscape.getProbability(a));
    }
    ~DecoredLandscape() {}

    const Graph & getNetwork() const { return network; }
    const QualityMap & getQualityMap() const { return qualityMap; }
    const CoordsMap & getCoordsMap() const { return coordsMap; }
    const ProbabilityMap & getProbabilityMap() const { return probabilityMap; }

    const double & getQuality(Node u) const { return qualityMap[u]; }
    const Point & getCoords(Node u) const { return coordsMap.get()[u]; }
    const double & getProbability(Arc a) const { return probabilityMap[a]; }

    double & getQualityRef(Node u) { return qualityMap[u]; }
    double & getProbabilityRef(Arc a) { return probabilityMap[a]; }

    void setQuality(Node u, double v) { qualityMap[u] = v; }
    void setProbability(Arc a, double v) { probabilityMap[a] = v; }

    /**
     * @brief Get the original quality of specified node.
     * @param u 
     * @return const double& 
     */
    const double & getOriginalQuality(Node u) const {
        return original_qualityMap.get()[u];
    }

    /**
     * @brief Get the original difficulty of specified arc.
     * @param u 
     * @return const double& 
     */
    const double & getOriginalProbability(Graph_t::Arc a) const {
        return original_probabilityMap.get()[a];
    }

    /**
     * @brief Resets the weights of the landscape to its original ones.
     */
    void reset() {
        for(typename Graph::NodeIt u(network); u != lemon::INVALID; ++u)
            setQuality(u, original_qualityMap.get()[u]);
        for(typename Graph::ArcIt a(network); a != lemon::INVALID; ++a)
            setProbability(a, original_probabilityMap.get()[a]);
    }

    void apply(const typename RestorationPlan<LS>::NodeEnhancements
                    & nodeEnhancements, const double coef = 1.0) {
        for(const auto & [u, quality_gain] : nodeEnhancements)
            qualityMap[u] =+ coef * quality_gain;
    }

    void apply(const typename RestorationPlan<LS>::ArcEnhancements
                    & arcEnhancements, const double coef = 1.0) {
        for(const auto & [a, restored_probability] : arcEnhancements) {
            probabilityMap[a] = std::max(probabilityMap[a],
                original_probabilityMap.get()[a] + coef *
                (restored_probability - original_probabilityMap.get()[a]));
        }
    }

    void apply(const typename RestorationPlan<LS>::NodeEnhancements
        & nodeEnhancements, const typename RestorationPlan<LS>::ArcEnhancements
        & arcEnhancements, const double coef = 1.0) {
        apply(nodeEnhancements, coef);
        apply(arcEnhancements, coef);
    }
};

#endif //DECORED_LANDSCAPE_HPP