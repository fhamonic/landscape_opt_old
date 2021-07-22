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

#include <fmt/os.h>

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


    template <typename Graph>
    class GraphToGraphviz {
    public:
        using NodePosMap = typename Graph::template NodeMap<lemon::dim2::Point<double>>;
        using NodeSizeMap = typename Graph::template NodeMap<double>;
        using NodeColorMap = typename Graph::template NodeMap<int>;
        using ArcSizeMap = typename Graph::template ArcMap<double>;
        using ArcColorMap = typename Graph::template ArcMap<int>;

        using Node = typename Graph::Node;
        using NodeIt = typename Graph::NodeIt;
        using Arc = typename Graph::Arc;
        using ArcIt = typename Graph::ArcIt;
    private:
        const Graph & _graph;
        std::filesystem::path _path;
        NodePosMap _nodePos;
        NodeSizeMap _nodeSizes;
        NodeColorMap _nodeColors;
        ArcSizeMap _arcSizes;
        ArcColorMap _arcColors;

        double _pageWidth;
        double _pageHeight;
    public:
        GraphToGraphviz(const Graph & g, const std::filesystem::path & p)
            : _graph(g)
            , _path(p)
            , _nodePos(g)
            , _nodeSizes(g, 1)
            , _nodeColors(g, 0xffffff)
            , _arcSizes(g, 1)
            , _arcColors(g, 0x000000)
            , _pageWidth(8)
            , _pageHeight(11) {}

        template <typename PM>
        GraphToGraphviz<Graph> & nodePos(const PM & posMap) {
            for(NodeIt u(_graph); u!=lemon::INVALID; ++u)
                _nodePos[u] = posMap[u];
            return *this;
        }
        template <typename SM>
        GraphToGraphviz<Graph> & nodeSizes(const SM & sizeMap) {
            for(NodeIt u(_graph); u!=lemon::INVALID; ++u)
                _nodeSizes[u] = sizeMap[u];
            return *this;
        }
        template <typename CM>
        GraphToGraphviz<Graph> & nodeColors(const CM & colorMap) {
            for(NodeIt u(_graph); u!=lemon::INVALID; ++u)
                _nodeColors[u] = colorMap[u];
            return *this;
        }
        template <typename AM>
        GraphToGraphviz<Graph> & arcSizes(const AM & sizeMap) {
            for(ArcIt a(_graph); a!=lemon::INVALID; ++a)
                _arcSizes[a] = sizeMap[a];
            return *this;
        }
        template <typename CM>
        GraphToGraphviz<Graph> & arcColors(const CM & colorMap) {
            for(ArcIt a(_graph); a!=lemon::INVALID; ++a)
                _arcColors[a] = colorMap[a];
            return *this;
        }
        GraphToGraphviz<Graph> & pageSize(double width, double height) {
            _pageWidth = width; _pageHeight = height;
            return *this;
        }


        void run() const {
            double min_x, max_x, min_y, max_y;
            min_x = max_x = _nodePos[_graph.nodeFromId(0)].x;
            min_y = max_y = _nodePos[_graph.nodeFromId(0)].y;
            for(NodeIt u(_graph); u!=lemon::INVALID; ++u) {
                min_x = std::min(min_x, _nodePos[u].x);
                max_x = std::max(max_x, _nodePos[u].x);
                min_y = std::min(min_y, _nodePos[u].y);
                max_y = std::max(max_y, _nodePos[u].y);
            }
            const double scale = std::min(_pageWidth/(max_x-min_x),
                                        _pageHeight/(max_y-min_y));
            auto scale_x = [&] (double x) { return scale * (x-min_x); };
            auto scale_y = [&] (double y) { return scale * (y-min_y); };
            auto scale_size = [&] (double s) { return scale * s; };

            std::vector<Node> colorSortedNodes;
            for(NodeIt u(_graph); u!=lemon::INVALID; ++u)
                colorSortedNodes.push_back(u);
            std::sort(colorSortedNodes.begin(), colorSortedNodes.end(), [&] (Node & a, Node & b) {
                return _nodeColors[a] < _nodeColors[b];
            });

            std::vector<Arc> colorSortedArcs;
            for(ArcIt a(_graph); a!=lemon::INVALID; ++a)
                colorSortedArcs.push_back(a);
            std::sort(colorSortedArcs.begin(), colorSortedArcs.end(), [&] (Arc & a, Arc & b) {
                return _arcColors[a] < _arcColors[b];
            });

            auto dot_file = fmt::output_file(_path.generic_string());
            dot_file.print("digraph {{size=\"{},{}\";\n", _pageWidth, _pageHeight);
            dot_file.print("graph [pad=\"0.2,0.1\" bgcolor=transparent overlap=scale]\n");
            dot_file.print("node [style=filled shape=\"circle\"]\n");
            dot_file.print("edge [style=filled]\n");

            int prev_color = -1;
            for(Node u : colorSortedNodes) {
                if(_nodeColors[u] != prev_color) {
                    dot_file.print("node [fillcolor=\"#{:06x}\"]\n", 
                        _nodeColors[u]);
                    prev_color = _nodeColors[u];
                }                
                dot_file.print("{} [area=\"{}\" pos=\"{},{}!\"]\n", 
                    _graph.id(u), scale_size(_nodeSizes[u]), 
                    scale_x(_nodePos[u].x), scale_y(_nodePos[u].y));
            }

            prev_color = -1;
            for(Arc a : colorSortedArcs) {
                if(_arcColors[a] != prev_color) {
                    dot_file.print("edge [color=\"#{:06x}\"]\n", 
                        _arcColors[a]);
                    prev_color = _arcColors[a];
                }
                dot_file.print("{} -> {} [penwidth=\"{}\"]\n", 
                    _graph.id(_graph.source(a)),
                    _graph.id(_graph.target(a)),
                    _arcSizes[a]);
            }

            dot_file.print("}}");
        }
    };

    template <typename LS>
    void printLandscapeGraphviz(const LS & landscape, std::filesystem::path path) {
        using Graph = typename LS::Graph;
        using Node = typename LS::Graph::Node;
        using NodeIt = typename LS::Graph::NodeIt;
        using Arc = typename LS::Graph::Arc;
        using ArcIt = typename LS::Graph::ArcIt;
        using NodeColorMap = typename Graph::template NodeMap<int>;
        using ArcColorMap = typename Graph::template ArcMap<int>;
        
        const Graph & graph = landscape.getNetwork();

        NodeColorMap nodeColors(graph);
        for(NodeIt u(graph); u!=lemon::INVALID; ++u)
            nodeColors[u] = landscape.getQuality(u)>0 ? 0x50e050 : 0x101010;

        GraphToGraphviz<Graph>(graph, path)
            .nodePos(landscape.getCoordsMap())
            .nodeSizes(landscape.getQualityMap())
            .nodeColors(nodeColors)
            .run();
    };

    template <typename LS>
    void printInstanceGraphviz(const LS & landscape, const RestorationPlan<LS> & plan, std::filesystem::path path) {
        using Graph = typename LS::Graph;
        using Node = typename LS::Graph::Node;
        using NodeIt = typename LS::Graph::NodeIt;
        using Arc = typename LS::Graph::Arc;
        using ArcIt = typename LS::Graph::ArcIt;
        using NodeColorMap = typename Graph::template NodeMap<int>;
        using ArcColorMap = typename Graph::template ArcMap<int>;
        
        const Graph & graph = landscape.getNetwork();

        NodeColorMap nodeColors(graph);
        for(NodeIt u(graph); u!=lemon::INVALID; ++u)
            nodeColors[u] = plan.contains(u) ? 0xe05050
                : (landscape.getQuality(u)>0 ? 0x50e050 : 0x101010);

        ArcColorMap arcColors(graph);
        for(ArcIt a(graph); a!=lemon::INVALID; ++a) {
            arcColors[a] = plan.contains(a) ? 0xe05050 : 0x101010;
            if(plan.contains(a)) {
                Node u = graph.source(a);
                Node v = graph.target(a);
                nodeColors[u] = (nodeColors[u] & 0xffff00) | 0x0000e0;
                nodeColors[v] = (nodeColors[v] & 0xffff00) | 0x0000e0;
            }
        }

        GraphToGraphviz<Graph>(graph, path)
            .nodePos(landscape.getCoordsMap())
            .nodeSizes(landscape.getQualityMap())
            .nodeColors(nodeColors)
            .arcColors(arcColors)
            .run();
    };


    template <typename LS>
    void printSolutionGraphviz(const LS & landscape, const RestorationPlan<LS> & plan, const Solution & solution, std::filesystem::path path) {
        using Graph = typename LS::Graph;
        using Node = typename LS::Graph::Node;
        using NodeIt = typename LS::Graph::NodeIt;
        using Arc = typename LS::Graph::Arc;
        using ArcIt = typename LS::Graph::ArcIt;
        using NodeColorMap = typename Graph::template NodeMap<int>;
        using ArcColorMap = typename Graph::template ArcMap<int>;
        
        const Graph & graph = landscape.getNetwork();

        NodeColorMap nodeColors(graph);
        for(NodeIt u(graph); u!=lemon::INVALID; ++u)
            nodeColors[u] = plan.contains(u) ? 0xe05050
                : (landscape.getQuality(u)>0 ? 0x50e050 : 0x101010);

        ArcColorMap arcColors(graph);
        for(ArcIt a(graph); a!=lemon::INVALID; ++a) {
            arcColors[a] = plan.contains(a) ? 0xe05050 : 0x101010;
            if(plan.contains(a)) {
                Node u = graph.source(a);
                Node v = graph.target(a);
                nodeColors[u] = (nodeColors[u] & 0xffff00) | 0x0000e0;
                nodeColors[v] = (nodeColors[v] & 0xffff00) | 0x0000e0;
            }
        }

        GraphToGraphviz<Graph>(graph, path)
            .nodePos(landscape.getCoordsMap())
            .nodeSizes(landscape.getQualityMap())
            .nodeColors(nodeColors)
            .arcSizes(Helper::arcCentralityMap(graph, landscape.getProbability))
            .arcColors(arcColors)
            .run();
    };

}


#endif // PRINT_HELPER_HPP