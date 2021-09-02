#ifndef ECA_HPP
#define ECA_HPP

#include "indices/concept/connectivity_index.hpp"
#include "algorithms/multiplicative_dijkstra.hpp"

class PartitionnedECA : public concepts::ConnectivityIndex {
public:
    PartitionnedECA() = default;
    ~PartitionnedECA() = default;

    /**
     * @brief Computes the value of the ECA index of the specified landscape
     * graph.
     *
     * @time \f$O(n \cdot (m + n) \log n)\f$ where \f$n\f$ is the number of
     * nodes and \f$m\f$ the number of arcs
     * @space \f$O(m)\f$ where \f$m\f$ is the number of arcs
     */
    template <typename GR, typename QM, typename PM>
    std::vector<typename GR::Node, double> eval(const GR & graph, const QM & qualityMap,
                const PM & probabilityMap) {
        lemon::MultiplicativeSimplerDijkstra<GR, PM> dijkstra(graph,
                                                              probabilityMap);
        double sum = 0;
        for(typename GR::NodeIt s(graph); s != lemon::INVALID; ++s) {
            if(qualityMap[s] == 0) continue;
            dijkstra.init(s);
            while(!dijkstra.emptyQueue()) {
                const std::pair<typename GR::Node, double> pair =
                    dijkstra.processNextNode();
                typename GR::Node t = pair.first;
                const double p_st = pair.second;
                sum += qualityMap[s] * qualityMap[t] * p_st;
            }
        }
        return std::sqrt(sum);
    }

    /**
     * @brief Computes the value of the ECA index of the specified landscape.
     *
     * @time \f$O(n \cdot (m + n) \log n)\f$ where \f$n\f$ is the number of
     * nodes and \f$m\f$ the number of arcs
     * @space \f$O(m)\f$ running and \f$O(1)\f$ returning where \f$m\f$ is the
     * number of arcs
     */
    template <typename LS>
    double eval(const LS & landscape) {
        return eval(landscape.getNetwork(), landscape.getQualityMap(),
                    landscape.getProbabilityMap());
    }
};

#endif  // ECA_HPP