/**
 * @file print_helper.hpp
 * @author francois.hamonic@gmail.com
 * @brief 
 * @version 0.1
 * @date 2021-07-19 
 */
#ifndef PRINT_HELPER_HPP
#define PRINT_HELPER_HPP

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

#include "helper.hpp"

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
        using Graph = typename LS::Graph;
        
        const Graph & graph = landscape.getNetwork();
        std::string name = path.stem();
        
        auto radius = [&] (double area) { return std::sqrt(area / (2*M_PI)); };
        const auto & [r_max, min_dist] = findNodeScale(landscape);
        const double a_max = r_max*2*radius(minNonZeroQuality(landscape));

        const bool directed = false;

        double node_scale = 0.7;
        double text_scale = 0.75 / (1+static_cast<int>(std::log10(lemon::countNodes(graph))));
        double arrow_scale = (1-node_scale)*2/3;
        double arc_width = node_scale * a_max / 16;
                
        typename Graph::template NodeMap<std::string> node_idsMap(graph, "");
        typename Graph::template NodeMap<lemon::Color> node_colorsMap(graph, lemon::BLACK);
        typename Graph::template NodeMap<double> node_sizesMap(graph, arc_width * 0.9);
        for(typename Graph::NodeIt v(graph); v != lemon::INVALID; ++v) {
            if(landscape.getQuality(v) == 0) continue;
            node_idsMap[v] = std::to_string(graph.id(v));
            node_colorsMap[v] = lemon::WHITE;
            node_sizesMap[v] = radius(landscape.getQuality(v));
        }

        typename Graph::template ArcMap<lemon::Color> arcs_colorsMap(graph, lemon::BLACK);
        typename Graph::template ArcMap<double> arc_widths(graph, arc_width);

        for(typename Graph::NodeIt u(graph); u != lemon::INVALID; ++u)
            node_colorsMap[u] = plan[u].empty() ? lemon::BLACK : lemon::RED;

        for(typename Graph::ArcIt a(graph); a != lemon::INVALID; ++a)
            arcs_colorsMap[a] = plan[a].empty() ? lemon::BLACK : lemon::RED;

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

    void printSolution(const Landscape & landscape,
            const RestorationPlan<Landscape>& plan,
            std::string name, concepts::Solver & solver,
            double B, const Solution & solution);

    
    // need to include the binary search tree for y-h , y+h search
    std::pair<Graph_t::Node, Graph_t::Node> neerestNodes(const Landscape & landscape);

    template <typename LS>
    void printLandscapeGraphviz(const LS & landscape, std::filesystem::path path) {
        using Graph = typename LS::Graph;
        using Node = typename LS::Graph::Node;
        using Arc = typename LS::Graph::Arc;
        using QualityMap = typename LS::QualityMap;
        using CoordsMap = typename LS::CoordsMap;
        
        const Graph & graph = landscape.getNetwork();
        const QualityMap & qualityMap = landscape.getQualityMap();
        const CoordsMap & coordsMap = landscape.getCoordsMap();

        double min_x, max_x, min_y, max_y;
        min_x = max_x = coordsMap[graph.nodeFromId(0)].x;
        min_y = max_y = coordsMap[graph.nodeFromId(0)].y;
        for(typename Graph::NodeIt u(graph); u!=lemon::INVALID; ++u) {
            min_x = std::min(min_x, coordsMap[u].x);
            max_x = std::max(max_x, coordsMap[u].x);
            min_y = std::min(min_y, coordsMap[u].y);
            max_y = std::max(max_y, coordsMap[u].y);
        }
        const double scale = std::min(8.3/(max_x-min_x), 11.7/(max_y-min_y));

        std::ofstream dot_file(path);

        dot_file << "digraph { size=\"8.3,11.7\";\n"
            << "graph [pad=\"0.212,0.055\" bgcolor=transparent overlap=scale]\n"
            << "node [style=filled fillcolor=\"#50e050\" shape=\"circle\"]\n";

        for(typename Graph::NodeIt u(graph); u!=lemon::INVALID; ++u) {
            dot_file << graph.id(u) << " [width=\"" << scale * std::sqrt(qualityMap[u])
                << "\" pos=\"" << scale * (coordsMap[u].x - min_x) << "," 
                << scale * (coordsMap[u].y - min_y) << "!\"]\n";
        }

        for(typename Graph::ArcIt a(graph); a!=lemon::INVALID; ++a) {
            dot_file << graph.id(graph.source(a)) << " -> "
                << graph.id(graph.target(a)) << "\n";
        }

        dot_file << "}" << std::endl;
    };

}


#endif // PRINT_HELPER_HPP