/**
 * @file helper.hpp
 * @author francois.hamonic@gmail.com
 * @brief 
 * @version 0.1
 * @date 2021-07-19 
 */
#ifndef HELPER_HPP
#define HELPER_HPP

#include <filesystem>
#include <math.h>

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>

#include "landscape/landscape.hpp"
#include "landscape/decored_landscape.hpp"

#include "Eigen/Dense"
#include "lemon/dijkstra.h"
#include "algorithms/multiplicative_dijkstra.hpp"

#include "solvers/concept/solver.hpp"
#include "fast-cpp-csv-parser/csv.h"

#include "lemon/graph_to_eps.h"
#include "lemon/dim2.h"

#include "indices/eca.hpp"

namespace Helper {
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
     * @brief Compute the distance matrix of a graph.
     * 
     * Generic implementation of computing the distance matrix of a graph by running dijkstra for each node.
     * The probability map values must support numeric operations.
     * Complexities are $O((m+n) \cdot \log n)$ time and $O(n^2)$ space, where $n$ is the number of nodes and $m$ the number of arcs.
     * Diagonal entries are $1$ and unreachable nodes entries are $0$.
     * 
     * @tparam GR : type of graph
     * @tparam DM : type of length map
     * @param g : graph
     * @param p : probability map
     * @return Eigen::Matrix<typename DM::Value, Eigen::Dynamic, Eigen::Dynamic>* 
     */
    template <typename GR, typename DM>
    static Eigen::Matrix<typename DM::Value, Eigen::Dynamic, Eigen::Dynamic> * multDistanceMatrix(const GR & g, DM & p) {
        typedef typename DM::Value Value;

        const int n = lemon::countNodes(g);
        Eigen::Matrix<Value,Eigen::Dynamic, Eigen::Dynamic> * distances = new Eigen::Matrix<Value, Eigen::Dynamic, Eigen::Dynamic>(n, n);
        distances->fill(0);
        
        lemon::MultiplicativeSimplerDijkstra<GR, DM> dijkstra(g, p);  
        
        for (typename GR::NodeIt s(g); s != lemon::INVALID; ++s) {
            const int id_s = g.id(s);
            dijkstra.run(s);

            for (typename GR::NodeIt t(g); t != lemon::INVALID; ++t) {
                const int id_t = g.id(t);
                Value & d_st = (*distances)(id_s, id_t);
                if(! dijkstra.reached(t)) { 
                    d_st = 0; 
                    continue;
                } 
                d_st = dijkstra.dist(t);
            }
        }
        return distances;
    }


    template <typename LS>
    double minNonZeroQuality(const LS & landscape) {
        using Graph = typename LS::Graph;
        const Graph & graph = landscape.getNetwork();
        double min = std::numeric_limits<double>::max();
        for(typename Graph::NodeIt v(graph); v != lemon::INVALID; ++v) {
            if(landscape.getQuality(v) == 0) continue;
            min = std::min(min, landscape.getQuality(v));
        }
        return min;
    }

    /**
     * @brief Compute the centrality of each arc, i.e the number of shortest paths that contain it
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
                if(!dijkstra.reached(t)) continue;
                typename GR::Node u = t;
                while(u != s) {
                    (*centralityMap)[predMap[t]] += 1;
                    u = graph.source(predMap[u]);
                }
            }
        }
        return centralityMap;
    }


    void printSolution(const Landscape & landscape,
            const RestorationPlan<Landscape>& plan,
            std::string name, concepts::Solver & solver,
            double B, const Solution & solution);

    
    template <typename LS_From, typename LS_To>
    void copyPlan(RestorationPlan<LS_To>& contracted_plan, const RestorationPlan<LS_From> & plan, 
            const typename LS_From::Graph::template NodeMap<typename LS_To::Node> & nodesRef, 
            const typename LS_From::Graph::template ArcMap<typename LS_To::Arc> & arcsRef) {
        assert(contracted_plan.getNbOptions() == 0);
        for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i)
            contracted_plan.addOption(plan.getCost(i));

        const typename LS_From::Graph & from_graph = plan.getLandscape().getNetwork();

        for(typename LS_From::Graph::NodeIt u(from_graph); u != lemon::INVALID; ++u)
            for(const auto & e : plan[u])
                contracted_plan.addNode(e.option, nodesRef[u], e.quality_gain);
        for(typename LS_From::Graph::ArcIt a(from_graph); a != lemon::INVALID; ++a)
            for(const auto & e : plan[a])
                contracted_plan.addArc(e.option, arcsRef[a], e.restored_probability);
    }

    // need to include the binary search tree for y-h , y+h search
    std::pair<Graph_t::Node, Graph_t::Node> neerestNodes(const Landscape & landscape);

    void assert_well_formed(const Landscape & landscape,
            const RestorationPlan<Landscape>& plan);
}


template <typename LS>
double max_flow_in(const LS & landscape, const RestorationPlan<LS>& plan, typename LS::Node t) {
    typedef typename LS::Graph Graph;
    typedef typename LS::ProbabilityMap ProbabilityMap;
    typedef lemon::ReverseDigraph<const Graph> Reversed;

    const Graph & original_g = landscape.getNetwork();
    Reversed reversed_g(original_g);
    ProbabilityMap probabilities(original_g);

    for(typename Graph::ArcIt b(original_g); b != lemon::INVALID; ++b) {
        probabilities[b] = landscape.getProbability(b);
        for(auto const & e : plan[b])
            probabilities[b] = std::max(probabilities[b],
                e.restored_probability);
    }

    lemon::MultiplicativeSimplerDijkstra<Reversed, ProbabilityMap> dijkstra(reversed_g, probabilities);
    double sum = 0;
    dijkstra.init(t);
    while(!dijkstra.emptyQueue()) {
        std::pair<typename Graph::Node, double> pair = dijkstra.processNextNode();
        typename Graph::Node v = pair.first;
        const double p_tv = pair.second;
        sum += landscape.getQuality(v) * p_tv;
        for(auto const & e : plan[v])
            sum += e.quality_gain * p_tv;
    }
    return sum;
}

#endif // HELPER