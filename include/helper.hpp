/**
 * @file helper.hpp
 * @author francois.hamonic@gmail.com
 * @brief 
 * @version 0.1
 * @date 2020-05-08 
 */
#ifndef HELPER_HPP
#define HELPER_HPP

#include <filesystem>
#include <math.h>

#include "landscape/landscape.hpp"
#include "landscape/decored_landscape.hpp"

#include "Eigen/Dense"
#include "lemon/dijkstra.h"

#include "solvers/concept/solver.hpp"
#include "fast-cpp-csv-parser/csv.h"

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
    std::pair<double, double> findNodeScale(const concepts::AbstractLandscape<GR, QM, DM, CM> & landscape) {
        const GR & graph = landscape.getNetwork();
        const CM & coordsMap = landscape.getCoordsMap();
        const QM & qualityMap = landscape.getQualityMap();

        if(lemon::countNodes(graph) < 2)
            return std::make_pair(1, 0);

        auto dist = [&] (typename GR::Node u, typename GR::Node v) {
            const double dx = coordsMap[u].x - coordsMap[v].x; 
            const double dy = coordsMap[u].y - coordsMap[v].y;        
            return std::sqrt(dx*dx + dy*dy);
        };
        auto radius = [&] (typename GR::Node u) {      
            return std::sqrt(qualityMap[u] / (2*M_PI));
        };
        double r_max = std::numeric_limits<double>::max();
        double min_dist = 0.0;
        for(typename GR::NodeIt u(graph); u != lemon::INVALID; ++u) {
            for(typename GR::NodeIt v(graph); v != lemon::INVALID; ++v) {
                if(u == v) continue;
                const double r = dist(u,v) / (radius(u) + radius(v));
                if(r < r_max) {
                    r_max = r;
                    min_dist = dist(u,v);
                }
            }
        }
        return std::make_pair(r_max, min_dist);
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


    template <typename LS>
    double minNonZeroQuality(const LS & landscape) {
        const Graph_t & graph = landscape.getNetwork();
        double min = std::numeric_limits<double>::max();
        for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
            if(landscape.getQuality(v) == 0) continue;
            min = std::min(min, landscape.getQuality(v));
        }
        return min;
    }


    template <typename LS>
    void printLandscape(const LS & landscape, std::filesystem::path path) {
        const Graph_t & graph = landscape.getNetwork();
        std::string name = path.stem();
        
        auto radius = [&] (double area) { return std::sqrt(area / (2*M_PI)); };
        const auto & [r_max, min_dist] = findNodeScale(landscape);
        const double a_max = r_max*2*radius(minNonZeroQuality(landscape));

        const bool directed = false;

        double node_scale = 0.7;
        double text_scale = 0.75 / (1+static_cast<int>(std::log10(lemon::countNodes(graph))));
        double arrow_scale = (1-node_scale)*2/3;
        double arc_width = node_scale * a_max / 16;
                
        Graph_t::NodeMap<std::string> node_idsMap(graph, "\7");
        Graph_t::NodeMap<lemon::Color> node_colorsMap(graph, lemon::WHITE);
        Graph_t::NodeMap<double> node_sizesMap(graph, arc_width * 0.9);
        for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
            if(landscape.getQuality(v) == 0) continue;
            node_idsMap[v] = std::to_string(graph.id(v));
            node_sizesMap[v] = radius(landscape.getQuality(v));
        }

        Graph_t::ArcMap<lemon::Color> arcs_colorsMap(graph, lemon::BLACK);
        Graph_t::ArcMap<double> arc_widths(graph, arc_width);

        return lemon::graphToEps(graph, "output/" + name)
                .title(name)
                .coords(landscape.getCoordsMap())
                .autoNodeScale(false)
                .absoluteNodeSizes(true)
                .nodeSizes(node_sizesMap)
                .nodeScale(node_scale * r_max)
                .autoArcWidthScale(false)
                .absoluteArcWidths(true)
                .arcWidths(arc_widths)
                .arcWidthScale(1)
                .drawArrows(directed)
                .arrowLength(arrow_scale * min_dist)
                .arrowWidth(arc_width * 2)
                .nodeTexts(node_idsMap)
                .nodeTextSize(text_scale * a_max)
                .nodeColors(node_colorsMap)
                .arcColors(arcs_colorsMap)
                .enableParallel(directed)
                .parArcDist(2 * arrow_scale * min_dist)
                .border(20)
                .run();
    }


    template <typename LS>
    void printInstance(const LS & landscape, const RestorationPlan<LS>& plan, std::filesystem::path path) {
        const Graph_t & graph = landscape.getNetwork();
        std::string name = path.stem();
        
        auto radius = [&] (double area) { return std::sqrt(area / (2*M_PI)); };
        const auto & [r_max, min_dist] = findNodeScale(landscape);
        const double a_max = r_max*2*radius(minNonZeroQuality(landscape));

        const bool directed = false;

        double node_scale = 0.7;
        double text_scale = 0.75 / (1+static_cast<int>(std::log10(lemon::countNodes(graph))));
        double arrow_scale = (1-node_scale)*2/3;
        double arc_width = node_scale * a_max / 16;
                
        Graph_t::NodeMap<std::string> node_idsMap(graph, "");
        Graph_t::NodeMap<lemon::Color> node_colorsMap(graph, lemon::BLACK);
        Graph_t::NodeMap<double> node_sizesMap(graph, arc_width * 0.9);
        for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
            if(landscape.getQuality(v) == 0) continue;
            node_idsMap[v] = std::to_string(graph.id(v));
            node_colorsMap[v] = lemon::WHITE;
            node_sizesMap[v] = radius(landscape.getQuality(v));
        }

        Graph_t::ArcMap<lemon::Color> arcs_colorsMap(graph, lemon::BLACK);
        Graph_t::ArcMap<double> arc_widths(graph, arc_width);

        for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i) {
            for(auto const& [u, quality_gain] : plan.getNodes(i))
                node_colorsMap[u] = lemon::RED;
            for(auto const& [a, restored_probability] : plan.getArcs(i))
                arcs_colorsMap[a] = lemon::RED;
        }

        return lemon::graphToEps(graph, path)
                .title(name)
                .coords(landscape.getCoordsMap())
                .autoNodeScale(false)
                .absoluteNodeSizes(true)
                .nodeSizes(node_sizesMap)
                .nodeScale(node_scale * r_max)
                .autoArcWidthScale(false)
                .absoluteArcWidths(true)
                .arcWidths(arc_widths)
                .arcWidthScale(1)
                .drawArrows(directed)
                .arrowLength(arrow_scale * min_dist)
                .arrowWidth(arc_width * 2)
                .nodeTexts(node_idsMap)
                .nodeTextSize(text_scale * a_max)
                .nodeColors(node_colorsMap)
                .arcColors(arcs_colorsMap)
                .enableParallel(directed)
                .parArcDist(2 * arrow_scale * min_dist)
                .border(2)
                .run();
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


    void printSolution(const Landscape & landscape, const RestorationPlan<Landscape>& plan, std::string name, concepts::Solver & solver, double B, Solution * solution);

    
    template <typename LS_From, typename LS_To>
    void copyPlan(RestorationPlan<LS_To>& contracted_plan, const RestorationPlan<LS_From>& plan, 
            const typename LS_From::Graph::template NodeMap<typename LS_To::Node> & nodesRef, 
            const typename LS_From::Graph::template ArcMap<typename LS_To::Arc> & arcsRef) {
        assert(contracted_plan.getNbOptions() == 0);
        for(typename RestorationPlan<LS_From>::Option i=0; i<plan.getNbOptions(); ++i) {
            contracted_plan.addOption(plan.getCost(i));
            for(auto const& [u, quality_gain] : plan.getNodes(i))
                contracted_plan.addNode(i, nodesRef[u], quality_gain);
            for(auto const& [a, restored_probability] : plan.getArcs(i))
                contracted_plan.addArc(i, arcsRef[a], restored_probability);
        }  
    }


    // need to include the binary search tree for y-h , y+h search
    std::pair<Graph_t::Node, Graph_t::Node> neerestNodes(const Landscape & landscape) ;


    void assert_well_formed(const Landscape & landscape, const RestorationPlan<Landscape>& plan);
}


template <typename LS>
double max_flow_in(const LS & landscape, const RestorationPlan<LS>& plan, typename LS::Node t) {
    typedef typename LS::Graph Graph;
    typedef typename LS::ProbabilityMap ProbabilityMap;
    typedef lemon::ReverseDigraph<const Graph> Reversed;

    const Graph & original_g = landscape.getNetwork();
    Reversed reversed_g(original_g);
    ProbabilityMap probabilities(original_g);

    for(typename Graph::ArcIt b(original_g); b != lemon::INVALID; ++b)
        probabilities[b] = landscape.getProbability(b);
    for(typename RestorationPlan<LS>::Option i=0; i<plan.getNbOptions(); ++i)
        for(auto const& [b, restored_probability] : plan.getArcs(i))
            probabilities[b] = std::max(probabilities[b], restored_probability);

    lemon::MultiplicativeSimplerDijkstra<Reversed, ProbabilityMap> dijkstra(reversed_g, probabilities);
    double sum = 0;
    dijkstra.init(t);
    while(!dijkstra.emptyQueue()) {
        std::pair<typename Graph::Node, double> pair = dijkstra.processNextNode();
        typename Graph::Node v = pair.first;
        const double p_tv = pair.second;
        sum += landscape.getQuality(v) * p_tv;
        for(auto const& [option, quality_gain] : plan.getOptions(v))
            sum += quality_gain * p_tv;
    }
    return sum;
}

#endif // HELPER