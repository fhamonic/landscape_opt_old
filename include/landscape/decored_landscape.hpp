/**
 * @file decored_landscape.hpp
 * @author François Hamonic (francois.hamonic@gmail.com)
 * @brief DecoredLandscape class declaration
 * @version 0.1
 * @date 2020-05-08
 */

#ifndef DECORED_LANDSCAPE_HPP
#define DECORED_LANDSCAPE_HPP

#include "lemon/list_graph.h"
#include "lemon/adaptors.h"
#include "lemon/maps.h"

#include "Eigen/Dense"

#include "landscape/concept/abstract_landscape.hpp"
#include "landscape/landscape.hpp"

#include "solvers/concept/restoration_plan.hpp"

/**
 * @brief Class that represent an decored landscape.
 * 
 * This class represent a decored landscape.
 * That is a modification of the weights of a reference landscape.
 */
class DecoredLandscape : public concepts::AbstractLandscape<Graph_t> {
    private:
        const Graph_t & network;
        const Graph_t::NodeMap<double> & original_qualityMap;
        const Graph_t::ArcMap<double> & original_probabilityMap;
        const CoordsMap & coordsMap;
        QualityMap qualityMap;
        ProbabilityMap probabilityMap;

    public:
        DecoredLandscape(const Graph_t & original_graph, const Graph_t::NodeMap<double> & original_qualityMap, const Graph_t::ArcMap<double> & original_probabilityMap, const CoordsMap & coordsMap);
        DecoredLandscape(const Landscape & landscape);
        ~DecoredLandscape();

        const Graph & getNetwork() const;
        const QualityMap & getQualityMap() const;
        const CoordsMap & getCoordsMap() const;
        const ProbabilityMap & getProbabilityMap() const;

        const double & getQuality(Graph_t::Node u) const;
        const Point & getCoords(Graph_t::Node u) const;
        const double & getProbability(Graph_t::Arc a) const;

        double & getQualityRef(Graph_t::Node u);
        double & getProbabilityRef(Graph_t::Arc a);

        void setQuality(Graph_t::Node u, double v);
        void setProbability(Graph_t::Arc e, double v);

        /**
         * @brief Get the original quality of specified node.
         * 
         * @param u 
         * @return const double& 
         */
        const double & getOriginalQuality(Graph_t::Node u) const;

        /**
         * @brief Get the original difficulty of specified arc.
         * 
         * @param u 
         * @return const double& 
         */
        const double & getOriginalProbability(Graph_t::Arc a) const;

        /**
         * @brief Resets the weights of the landscape to its original ones.
         */
        void reset();

        /**
         * @brief Decorates the landscape by applying the specified restoration option.
         * 
         * Enhances the nodes qualities and arcs probabilities concerned by the option **i** of the restoration plan.
         * **coef** permits to partially apply the restoration option.
         * 
         * @param plan - RestorationPlan
         * @param option - Option
         * @param coef - $[0,1]$, the portion of option to consider  
         */
        void apply(const RestorationPlan<Landscape>& plan, RestorationPlan<Landscape>::Option i, double coef=1.0);
};

#endif //DECORED_LANDSCAPE_HPP