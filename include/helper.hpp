/**
 * @file helper.hpp
 * @author francois.hamonic@gmail.com
 * @brief
 * @version 0.1
 * @date 2021-07-19
 */
#ifndef HELPER_HPP
#define HELPER_HPP

#include <math.h>
#include <filesystem>
#include <iterator>

#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>

#include "landscape/decored_landscape.hpp"
#include "landscape/mutable_landscape.hpp"

#include "Eigen/Dense"
#include "algorithms/multiplicative_dijkstra.hpp"
#include "lemon/dijkstra.h"

#include "fast-cpp-csv-parser/csv.h"
#include "solvers/concept/solver.hpp"

#include "lemon/dim2.h"
#include "lemon/graph_to_eps.h"

#include "indices/eca.hpp"

namespace Helper {
/**
 * @brief Compute the distance matrix of a graph.
 *
 * Generic implementation of computing the distance matrix of a graph by running
 * dijkstra for each node. The length map values must support numeric
 * operations. Complexities are $O((m+n) \cdot \log n)$ time and $O(n^2)$ space,
 * where $n$ is the number of nodes and $m$ the number of arcs. Diagonal entries
 * are $0$ and unreachable nodes entries are
 * $std::numeric_limits<Value>::max()$.
 *
 * @tparam GR : type of graph
 * @tparam DM : type of length map
 * @param g : graph
 * @param l : length map
 * @return Eigen::Matrix<typename DM::Value, Eigen::Dynamic, Eigen::Dynamic>*
 */
template <typename GR, typename DM>
static Eigen::Matrix<typename DM::Value, Eigen::Dynamic, Eigen::Dynamic> *
distanceMatrix(const GR & g, DM & l) {
    using Value = typename DM::Value;

    const int n = lemon::countNodes(g);
    Eigen::Matrix<Value, Eigen::Dynamic, Eigen::Dynamic> * distances =
        new Eigen::Matrix<Value, Eigen::Dynamic, Eigen::Dynamic>(n, n);
    distances->fill(std::numeric_limits<Value>::max());

    lemon::SimplerDijkstra<GR, DM> dijkstra(g, l);

    for(typename GR::NodeIt s(g); s != lemon::INVALID; ++s) {
        const int id_s = g.id(s);
        dijkstra.run(s);

        for(typename GR::NodeIt t(g); t != lemon::INVALID; ++t) {
            const int id_t = g.id(t);
            Value & d_st = (*distances)(id_s, id_t);
            if(!dijkstra.reached(t)) {
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
 * Generic implementation of computing the distance matrix of a graph by running
 * dijkstra for each node. The probability map values must support numeric
 * operations. Complexities are $O((m+n) \cdot \log n)$ time and $O(n^2)$ space,
 * where $n$ is the number of nodes and $m$ the number of arcs. Diagonal entries
 * are $1$ and unreachable nodes entries are $0$.
 *
 * @tparam GR : type of graph
 * @tparam DM : type of length map
 * @param g : graph
 * @param p : probability map
 * @return Eigen::Matrix<typename DM::Value, Eigen::Dynamic, Eigen::Dynamic>*
 */
template <typename GR, typename DM>
static Eigen::Matrix<typename DM::Value, Eigen::Dynamic, Eigen::Dynamic> *
multDistanceMatrix(const GR & g, DM & p) {
    using Value = typename DM::Value;

    const int n = lemon::countNodes(g);
    Eigen::Matrix<Value, Eigen::Dynamic, Eigen::Dynamic> * distances =
        new Eigen::Matrix<Value, Eigen::Dynamic, Eigen::Dynamic>(n, n);
    distances->fill(0);

    lemon::MultiplicativeSimplerDijkstra<GR, DM> dijkstra(g, p);

    for(typename GR::NodeIt s(g); s != lemon::INVALID; ++s) {
        const int id_s = g.id(s);
        dijkstra.run(s);

        for(typename GR::NodeIt t(g); t != lemon::INVALID; ++t) {
            const int id_t = g.id(t);
            Value & d_st = (*distances)(id_s, id_t);
            if(!dijkstra.reached(t)) {
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
 * @brief Compute the centrality of each arc, i.e the number of shortest paths
 * that contain it
 * @tparam GR
 * @tparam LM
 * @param graph
 * @param lengthMap
 * @return GR::ArcMap<int>*
 */
template <typename Graph, typename LengthMap>
std::unique_ptr<typename Graph::ArcMap<int>> arcCentralityMap(
    const Graph & graph, const LengthMap & lengthMap) {
    using PredMap = typename Graph::template NodeMap<typename Graph::Arc>;

    std::unique_ptr<typename Graph::ArcMap<int>> centralityMap =
        std::make_unique<typename Graph::ArcMap<int>>(graph, 0);

    lemon::Dijkstra<Graph, LengthMap> dijkstra(graph, lengthMap);

    for(typename Graph::NodeIt s(graph); s != lemon::INVALID; ++s) {
        dijkstra.run(s);
        const PredMap & predMap = dijkstra.predMap();
        for(typename Graph::NodeIt t(graph); t != lemon::INVALID; ++t) {
            if(!dijkstra.reached(t)) continue;
            typename Graph::Node u = t;
            while(u != s) {
                (*centralityMap)[predMap[u]] += 1;
                u = graph.source(predMap[u]);
            }
        }
    }
    return centralityMap;
}

/**
 * @brief Compute the centrality of each arc, i.e the number of shortest paths
 * that contain it
 * @tparam GR
 * @tparam LM
 * @param graph
 * @param lengthMap
 * @return GR::ArcMap<int>*
 */
template <typename LS>
std::unique_ptr<typename LS::Graph::ArcMap<double>> corridorCentralityMap(
    const LS & landscape) {
    using Graph = typename LS::Graph;
    using PredMap = typename Graph::template NodeMap<typename Graph::Arc>;
    const Graph & graph = landscape.getNetwork();

    std::unique_ptr<typename Graph::ArcMap<double>> centralityMap =
        std::make_unique<typename Graph::ArcMap<double>>(graph, 0);

    lemon::Dijkstra<Graph, typename LS::ProbabilityMap> dijkstra(
        graph, landscape.getProbabilityMap());

    for(typename Graph::NodeIt s(graph); s != lemon::INVALID; ++s) {
        if(landscape.getQuality(s) == 0) continue;
        dijkstra.run(s);
        const PredMap & predMap = dijkstra.predMap();
        for(typename Graph::NodeIt t(graph); t != lemon::INVALID; ++t) {
            if(landscape.getQuality(t) == 0) continue;
            if(!dijkstra.reached(t)) continue;
            typename Graph::Node u = t;
            while(u != s) {
                (*centralityMap)[predMap[u]] += 1;
                u = graph.source(predMap[u]);
            }
        }
    }
    return centralityMap;
}

/**
 * @brief Computes the value of the ECA index of the specified landscape
 * graph.
 *
 * @time \f$O(n \cdot (m + n) \log n)\f$ where \f$n\f$ is the number of
 * nodes and \f$m\f$ the number of arcs
 * @space \f$O(m)\f$ where \f$m\f$ is the number of arcs
 */
template <typename GR, typename QM, typename PM>
std::vector<std::pair<typename GR::Node, double>> computeDistancePairs(
    const GR & graph, const QM & qualityMap, const PM & probabilityMap,
                             const typename GR::Node s) {
    using Node = typename GR::Node;

    std::vector<std::pair<Node, double>> result;
    result.reserve(lemon::countNodes(graph));
    lemon::MultiplicativeSimplerDijkstra<GR, PM> dijkstra(graph,
                                                          probabilityMap);
    if(qualityMap[s] != 0) {
        dijkstra.init(s);
        while(!dijkstra.emptyQueue())
            result.emplace_back(dijkstra.processNextNode());
    }
    return result;
}

template <typename LS>
std::vector<std::pair<typename LS::Node, double>> computeDistancePairs(const LS & landscape,
                             const typename LS::Node s) {
    auto result = computeDistancePairs(landscape.getNetwork(),
                                          landscape.getQualityMap(),
                                          landscape.getProbabilityMap(), s);
    return result;
}


template <typename LS>
DecoredLandscape<LS> decore_landscape(const LS & landscape,
                                      const RestorationPlan<LS> & plan,
                                      const Solution & solution) {
    using Graph = typename LS::Graph;
    const Graph & graph = landscape.getNetwork();
    DecoredLandscape<LS> decored_landscape(landscape);

    for(typename Graph::NodeIt u(graph); u != lemon::INVALID; ++u)
        for(const auto & e : plan[u])
            decored_landscape.getQualityRef(u) +=
                solution[e.option] * e.quality_gain;
    for(typename Graph::ArcIt a(graph); a != lemon::INVALID; ++a)
        for(const auto & e : plan[a])
            decored_landscape.getProbabilityRef(a) = std::max(
                decored_landscape.getProbability(a),
                landscape.getProbability(a) +
                    solution[e.option] *
                        (e.restored_probability - landscape.getProbability(a)));

    return decored_landscape;
}

void printSolution(const MutableLandscape & landscape,
                   const RestorationPlan<MutableLandscape> & plan,
                   std::string name, concepts::Solver & solver, double B,
                   const Solution & solution);

template <typename LS_From, typename LS_To>
void copyPlan(
    RestorationPlan<LS_To> & contracted_plan,
    const RestorationPlan<LS_From> & plan,
    const typename LS_From::Graph::template NodeMap<typename LS_To::Node> &
        nodesRef,
    const typename LS_From::Graph::template ArcMap<typename LS_To::Arc> &
        arcsRef) {
    assert(contracted_plan.getNbOptions() == 0);
    for(RestorationPlan<MutableLandscape>::Option i = 0;
        i < plan.getNbOptions(); ++i)
        contracted_plan.addOption(plan.getCost(i));

    const typename LS_From::Graph & from_graph =
        plan.getLandscape().getNetwork();

    for(typename LS_From::Graph::NodeIt u(from_graph); u != lemon::INVALID; ++u)
        for(const auto & e : plan[u])
            contracted_plan.addNode(e.option, nodesRef[u], e.quality_gain);
    for(typename LS_From::Graph::ArcIt a(from_graph); a != lemon::INVALID; ++a)
        for(const auto & e : plan[a])
            contracted_plan.addArc(e.option, arcsRef[a],
                                   e.restored_probability);
}

// need to include the binary search tree for y-h , y+h search
std::pair<MutableLandscape::Node, MutableLandscape::Node> neerestNodes(
    const MutableLandscape & landscape);

void assert_well_formed(const MutableLandscape & landscape,
                        const RestorationPlan<MutableLandscape> & plan);

// template <typename GR>
// class NodeIterator {
// public:
//     using Node = typename GR::Node;
//     using NodeIt = typename GR::NodeIt;

//     using difference_type = std::ptrdiff_t;
//     using value_type = Node;
//     using pointer = void;
//     using reference = const Node&;
//     using iterator_category = std::input_iterator_tag;

// private:
//     NodeIt it;
// public:
//     NodeIterator() : it(lemon::INVALID) {}
//     NodeIterator(const GR & graph) : it(graph) {}
//     NodeIterator(const NodeIterator<GR> & o) : it(o.it) {}

//     NodeIterator& operator=(const NodeIterator<GR> & o) { it = o.it; return
//     *this; }

//     value_type operator*() const { return static_cast<value_type>(it); }
//     NodeIterator operator++(int) const { return ++NodeIterator<GR>(*this); }
//     NodeIterator& operator++() { ++it; return *this; }

//     bool operator==(const NodeIterator<GR> & o) const { return o.it == it; }
//     bool operator!=(const NodeIterator<GR> & o) const { return o.it != it; }
// };

// template <typename GR>
// std::pair<NodeIterator<GR>, NodeIterator<GR>> nodesRange(const GR & graph) {
//     return std::make_pair(NodeIterator(graph), NodeIterator<GR>());
// }
}  // namespace Helper

template <typename LS>
double max_flow_in(const LS & landscape, const RestorationPlan<LS> & plan,
                   typename LS::Node t) {
    using Graph = typename LS::Graph;
    using ProbabilityMap = typename LS::ProbabilityMap;
    using Reversed = lemon::ReverseDigraph<const Graph>;

    const Graph & original_g = landscape.getNetwork();
    Reversed reversed_g(original_g);
    ProbabilityMap probabilities(original_g);

    for(typename Graph::ArcIt b(original_g); b != lemon::INVALID; ++b) {
        probabilities[b] = landscape.getProbability(b);
        for(auto const & e : plan[b])
            probabilities[b] =
                std::max(probabilities[b], e.restored_probability);
    }

    lemon::MultiplicativeSimplerDijkstra<Reversed, ProbabilityMap> dijkstra(
        reversed_g, probabilities);
    double sum = 0;
    dijkstra.init(t);
    while(!dijkstra.emptyQueue()) {
        std::pair<typename Graph::Node, double> pair =
            dijkstra.processNextNode();
        typename Graph::Node v = pair.first;
        const double p_tv = pair.second;
        sum += landscape.getQuality(v) * p_tv;
        for(auto const & e : plan[v]) sum += e.quality_gain * p_tv;
    }
    return sum;
}

#endif  // HELPER