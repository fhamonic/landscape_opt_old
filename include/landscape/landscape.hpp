/**
 * @file landscape.hpp
 * @author François Hamonic (francois.hamonic@gmail.com)
 * @brief Landscape class declaration
 * @version 0.1
 * @date 2020-05-08
 */

#ifndef LANDSCAPE_HPP
#define LANDSCAPE_HPP

#include "lemon/list_graph.h"
#include "lemon/adaptors.h"
#include "lemon/maps.h"

#include "Eigen/Dense"

#include "landscape/concept/abstract_landscape.hpp"


/**
 * @brief Class that represent an editable landscape.
 * 
 * This class represent an editable landscape.
 * That is a graph whose nodes weigths are quality and coordinates, and arcs weigths are probabilities to success crossing. 
 */
class Landscape : public concepts::AbstractLandscape<Graph_t> {
    private:
        Graph network;
        QualityMap qualityMap;
        CoordsMap coordsMap;
        ProbabilityMap probabilityMap;

    public:
        Landscape();
        Landscape(const Landscape&);
        Landscape(Landscape&&);
        ~Landscape();
        Landscape & operator=(const Landscape&) { assert(false && "Fuck"); return *this; };
        Landscape & operator=(Landscape&&) { assert(false && "Fuck"); return *this; };

        /**
         * @brief Makes the current landscape a copy of the one passed in parameter.
         * 
         * @param orig_landscape : the landscape to copy 
         * @return a pair of references maps from original landscape elements to the copied ones 
         */
        std::pair<Graph_t::NodeMap<Node>*, Graph_t::ArcMap<Arc>*> copy(const Landscape & orig_landscape);

        /**
         * @brief Adds a new patch with specified quality and coordinates.
         * 
         * @param quality 
         * @param coords 
         * @return the created graph Node 
         */
        Node addNode(double quality, Point coords);

        /**
         * @brief Adds a new arc from u to v with specified probability.
         * 
         * @param u 
         * @param v
         * @param difficulty 
         * @return the created graph Arc 
         */
        Arc addArc(Node u, Node v, double difficulty);

        /**
         * @brief Removes the node u.
         * 
         * @param u 
         */
        void removeNode(Node u);

        /**
         * @brief Removes the arc a.
         * 
         * @param a 
         */
        void removeArc(Arc a);

        /**
         * @brief Changes the source of the arc a to be u
         * 
         * @param a 
         * @param u 
         */
        void changeSource(Arc a, Node u);

        /**
         * @brief Changes the source of the arc a to be v
         * 
         * @param a 
         * @param v 
         */
        void changeTarget(Arc a, Node v);

        /**
         * @brief Set the quality of the node u
         * 
         * @param u 
         * @param quality 
         */
        void setQuality(Node u, double quality);
        /**
         * @brief Set the coordinates of the node u
         * 
         * @param u 
         * @param coords 
         */
        void setCoords(Node u, Point coords);

        /**
         * @brief Set the probability of the arc a
         * 
         * @param a
         * @param difficulty 
         */
        void setProbability(Arc a, double probability);

        const Graph & getNetwork() const;
        const QualityMap & getQualityMap() const;
        const CoordsMap & getCoordsMap() const;
        const ProbabilityMap & getProbabilityMap() const;

        const double & getQuality(Node u) const;
        const Point & getCoords(Node u) const;
        const double & getProbability(Arc a) const;

        double & getQualityRef(Node u);
        Point & getCoordsRef(Node u);
        double & getProbabilityRef(Arc a);
};

#endif //LANDSCAPE_HPP