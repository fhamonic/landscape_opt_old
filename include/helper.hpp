/**
 * @file helper.hpp
 * @author francois.hamonic@gmail.com
 * @brief 
 * @version 0.1
 * @date 2020-05-08 
 */
#ifndef HELPER_HPP
#define HELPER_HPP

#include <math.h>

#include "landscape/landscape.hpp"
#include "landscape/decored_landscape.hpp"

#include "Eigen/Dense"
#include "lemon/dijkstra.h"

#include "solvers/concept/solver.hpp"
#include "csv.hpp"

#include "lemon/graph_to_eps.h"
#include "lemon/dim2.h"

#include "indices/eca.hpp"

namespace Helper {
    /**
     * @brief Finds the maximum node scaling for which no nodes overlaps.
     * 
     * Finds the maximum scaling coeficient of nodes radius for which their is no overlaps in their graphical representation.
     * Runs in $O(n^2)$, where $n$ is the number of nodes.
     * 
     * @tparam GR 
     * @tparam QM 
     * @tparam DM 
     * @tparam CM 
     * @param landscape 
     * @return double 
     */
    template <typename GR, typename QM, typename DM, typename CM>
    double findNodeScale(const concepts::AbstractLandscape<GR, QM, DM, CM> & landscape) {
        const GR & graph = landscape.getNetwork();
        const CM & coordsMap = landscape.getCoordsMap();
        const QM & qualityMap = landscape.getQualityMap();

        auto dist = [&] (typename GR::Node u, typename GR::Node v) {
            const double dx = coordsMap[u].x - coordsMap[v].x; 
            const double dy = coordsMap[u].y - coordsMap[v].y;        
            return std::sqrt(dx*dx + dy*dy);
        };
        auto radius = [&] (typename GR::Node u) {      
            return std::sqrt(qualityMap[u] / (2*M_PI));
        };
        double r_max = std::numeric_limits<double>::max();
        for(typename GR::NodeIt u(graph); u != lemon::INVALID; ++u) {
            for(typename GR::NodeIt v(graph); v != lemon::INVALID; ++v) {
                if(u == v) 
                    continue;
                const double r = dist(u,v) / (radius(u) + radius(v));
                r_max = std::min(r_max, r);
            }
        }
        return r_max;
    }

    /**
     * @brief Compute the distance matrix of a graph.
     * 
     * Generic implementation of computing the distance matrix of a graph by running dijkstra for each node.
     * The length map values must support numeric operations.
     * Complexities are $O((m+n) \cdot \log n)$ time and $O(n^2)$ space, where $n$ is the number of nodes and $m$ the number of arcs.
     * Diagonal entries are $0$ and unreachable nodes entries are $std::numeric_limits<Value>::max()$.
     * 
     * @tparam GR : type of graph
     * @tparam DM : type of length map
     * @param g : graph
     * @param l : length map
     * @return Eigen::Matrix<typename DM::Value, Eigen::Dynamic, Eigen::Dynamic>* 
     */
    template <typename GR, typename DM>
    static Eigen::Matrix<typename DM::Value, Eigen::Dynamic, Eigen::Dynamic> * distanceMatrix(const GR & g, DM & l) {
        typedef typename DM::Value Value;

        const int n = lemon::countNodes(g);
        Eigen::Matrix<Value,Eigen::Dynamic, Eigen::Dynamic> * distances = new Eigen::Matrix<Value, Eigen::Dynamic, Eigen::Dynamic>(n, n);
        distances->fill(std::numeric_limits<Value>::max());
        
        lemon::SimplerDijkstra<GR, DM> dijkstra(g, l);  
        
        for (typename GR::NodeIt s(g); s != lemon::INVALID; ++s) {
            const int id_s = g.id(s);
            dijkstra.run(s);

            for (typename GR::NodeIt t(g); t != lemon::INVALID; ++t) {
                const int id_t = g.id(t);
                Value & d_st = (*distances)(id_s, id_t);
                if(s == t) { 
                    d_st = 0; 
                    continue;
                }
                if(! dijkstra.reached(t)) { 
                    d_st = std::numeric_limits<Value>::max(); 
                    continue;
                } 
                d_st = dijkstra.dist(t);
            }
        }
        return distances;
    }

    /**
     * @brief Compute the centrality of each arc, i.e the number of shortest paths that contain it
     * 
     * @tparam GR 
     * @tparam LM 
     * @param graph 
     * @param lengthMap 
     * @return Graph_t::ArcMap<int>* 
     */
    template <typename GR, typename LM>
    Graph_t::ArcMap<int> * arcCentralityMap(const GR & graph, const LM & lengthMap) {
        typedef GR Graph;
        typedef LM LengthMap;
        typedef typename GR::template NodeMap<typename GR::Arc> PredMap;

        Graph_t::ArcMap<int> * centralityMap = new Graph_t::ArcMap<int>(graph, 0);
        
        lemon::Dijkstra<Graph, LengthMap> dijkstra(graph, lengthMap);
            
        for (typename Graph::NodeIt s(graph); s != lemon::INVALID; ++s) {
            dijkstra.run(s);

            const PredMap & predMap = dijkstra.predMap();

            for (typename Graph::NodeIt t(graph); t != lemon::INVALID; ++t) {
                if(! dijkstra.reached(t))
                    continue;

                typename GR::Node u = t;
                while(u != s) {
                    (*centralityMap)[predMap[t]] += 1;
                    u = graph.source(predMap[u]);
                }
            }
        }
        return centralityMap;
    }


    void printSolution(const Landscape & landscape, const RestorationPlan & plan, std::string name, concepts::Solver & solver, double B, Solution * solution);

    void copyPlan(RestorationPlan & contracted_plan, const RestorationPlan & plan, const Graph_t::NodeMap<Graph_t::Node> & nodesRef, const Graph_t::ArcMap<Graph_t::Arc> & arcsRef);

    // need to include the binary search tree for y-h , y+h search
    std::pair<Graph_t::Node, Graph_t::Node> neerestNodes(const Landscape & landscape) ;


    void assert_well_formed(const Landscape & landscape, const RestorationPlan & plan);
}

#endif // HELPER