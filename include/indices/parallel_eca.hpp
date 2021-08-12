#ifndef PARALLEL_ECA_HPP
#define PARALLEL_ECA_HPP

#include <algorithm>
#include <execution>

#include "indices/concept/connectivity_index.hpp"

#include "algorithms/multiplicative_dijkstra.hpp"

#include "lemon/bits/map_extender.h"

#include "landscape/decored_landscape.hpp"
#include "solvers/concept/solution.hpp"

class Parallel_ECA : public concepts::ConnectivityIndex {
public:
    Parallel_ECA(){};
    ~Parallel_ECA(){};

    static double P_func(const double d, const double alpha) {
        assert(d >= 0);
        return std::exp(-d / alpha);
    }

    static double inv_P_func(const double p, const double alpha) {
        assert(p >= 0 && p <= 1);
        return -alpha * std::log(p);
    }

    /**
     * @brief Computes the value of the Parallel_ECA index of the specified
     * landscape.
     *
     * @time \f$O(n \cdot (m + n) \log n)\f$ where \f$n\f$ is the number of
     * nodes and \f$m\f$ the number of arcs
     * @space \f$O(m)\f$ where \f$m\f$ is the number of arcs
     */
    template <typename LS>
    double eval(const LS & landscape,
                const typename LS::Graph::template NodeMap<bool> & nodeFilter) {
        const typename LS::Graph & g = landscape.getNetwork();
        const typename LS::QualityMap & qualityMap = landscape.getQualityMap();
        const typename LS::ProbabilityMap & probabilityMap =
            landscape.getProbabilityMap();

        std::vector<typename LS::Node> nodes;
        for(typename LS::NodeIt s(g); s != lemon::INVALID; ++s) {
            if(!nodeFilter[s] || qualityMap[s] == 0) continue;
            nodes.push_back(s);
        }

        return std::sqrt(std::transform_reduce(
            std::execution::par_unseq, nodes.begin(), nodes.end(), 0.0,
            std::plus<>(), [&](typename LS::Node s) {
                double sum = 0;
                lemon::MultiplicativeSimplerDijkstra<
                    typename LS::Graph, typename LS::ProbabilityMap>
                    dijkstra(g, probabilityMap);
                dijkstra.init(s);
                while(!dijkstra.emptyQueue()) {
                    std::pair<typename LS::Node, double> pair =
                        dijkstra.processNextNode();
                    typename LS::Node t = pair.first;
                    if(!nodeFilter[t]) continue;
                    const double p_st = pair.second;
                    sum += qualityMap[s] * qualityMap[t] * p_st;
                }
                return sum;
            }));
    }

    /**
     * @brief Computes the value of the Parallel_ECA index of the specified
     * landscape.
     *
     * @time \f$O(n \cdot (m + n) \log n)\f$ where \f$n\f$ is the number of
     * nodes and \f$m\f$ the number of arcs
     * @space \f$O(m)\f$ running and \f$O(1)\f$ returning where \f$m\f$ is the
     * number of arcs
     */
    template <typename LS>
    double eval(const LS & landscape) {
        const typename LS::Graph & g = landscape.getNetwork();
        const typename LS::QualityMap & qualityMap = landscape.getQualityMap();
        const typename LS::ProbabilityMap & probabilityMap =
            landscape.getProbabilityMap();

        std::vector<typename LS::Node> nodes;
        for(typename LS::NodeIt s(g); s != lemon::INVALID; ++s) {
            if(qualityMap[s] == 0) continue;
            nodes.push_back(s);
        }

        return std::sqrt(std::transform_reduce(
            std::execution::par_unseq, nodes.begin(), nodes.end(), 0.0,
            std::plus<>(), [&](typename LS::Node s) {
                double sum = 0;
                lemon::MultiplicativeSimplerDijkstra<
                    typename LS::Graph, typename LS::ProbabilityMap>
                    dijkstra(g, probabilityMap);
                dijkstra.init(s);
                while(!dijkstra.emptyQueue()) {
                    std::pair<typename LS::Node, double> pair =
                        dijkstra.processNextNode();
                    typename LS::Node t = pair.first;
                    const double p_st = pair.second;
                    sum += qualityMap[s] * qualityMap[t] * p_st;
                }
                return sum;
            }));
    }
};

#endif  // Parallel_ECA_HPP