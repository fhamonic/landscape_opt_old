#ifndef ECA_HPP
#define ECA_HPP

#include "indices/concept/connectivity_index.hpp"

#include "algorithms/multiplicative_dijkstra.h"

#include "lemon/bits/map_extender.h"

#include "solvers/concept/solution.hpp"
#include "landscape/decored_landscape.hpp"

class ECA : public concepts::ConnectivityIndex {
    private:
        static ECA singleton;
        ECA();
    public:
        static ECA & get() noexcept {
            return singleton;
        }
        ~ECA();
   
        static double P_func(const double d, const double alpha) {
            assert(d >= 0);
            return std::exp(-d/alpha);
        }

        static double inv_P_func(const double p, const double alpha) {
            assert(p >= 0 && p <= 1);
            return -alpha * std::log(p);
        }

        double eval_solution(const Landscape & landscape, const Solution & solution) const;


        /**
         * @brief Computes the value of the ECA index of the specified landscape.
         * 
         * @time \f$O(n \cdot (m + n) \log n)\f$ where \f$n\f$ is the number of nodes and \f$m\f$ the number of arcs
         * @space \f$O(m)\f$ where \f$m\f$ is the number of arcs
         */
        template <typename GR, typename QM, typename PM, typename CM>
        double eval(const concepts::AbstractLandscape<GR, QM, PM, CM> & landscape, const typename GR::template NodeMap<bool> & nodeFilter) {
            const GR & g = landscape.getNetwork();
            const QM & qualityMap = landscape.getQualityMap();
            const PM & probabilityMap = landscape.getProbabilityMap();
            
            lemon::MultiplicativeSimplerDijkstra<GR, PM> dijkstra(g, probabilityMap);
            double sum = 0;
            for (typename GR::NodeIt s(g); s != lemon::INVALID; ++s) {
                if(!nodeFilter[s])
                    continue;
                dijkstra.init(s);
                while (!dijkstra.emptyQueue()) {
                    std::pair<typename GR::Node, double> pair = dijkstra.processNextNode();
                    typename GR::Node t = pair.first;
                    if(!nodeFilter[t])
                        continue;
                    const double p_st = pair.second;
                    sum += qualityMap[s] * qualityMap[t] * p_st;
                }
            }
            return std::sqrt(sum);
        }

        /**
         * @brief Computes the value of the ECA index of the specified landscape.
         * 
         * @time \f$O(n \cdot (m + n) \log n)\f$ where \f$n\f$ is the number of nodes and \f$m\f$ the number of arcs
         * @space \f$O(m)\f$ running and \f$O(1)\f$ returning where \f$m\f$ is the number of arcs
         */
        template <typename GR, typename QM, typename PM, typename CM>
        double eval(const concepts::AbstractLandscape<GR, QM, PM, CM> & landscape) {
            const GR & g = landscape.getNetwork();
            const QM & qualityMap = landscape.getQualityMap();
            const PM & probabilityMap = landscape.getProbabilityMap();
            
            lemon::MultiplicativeSimplerDijkstra<GR, PM> dijkstra(g, probabilityMap);
            double sum = 0;
            for (typename GR::NodeIt s(g); s != lemon::INVALID; ++s) {
                dijkstra.init(s);
                while (!dijkstra.emptyQueue()) {
                    std::pair<typename GR::Node, double> pair = dijkstra.processNextNode();
                    typename GR::Node t = pair.first;
                    const double p_st = pair.second;
                    sum += qualityMap[s] * qualityMap[t] * p_st;
                }
            }
            return std::sqrt(sum);
        }
};

#endif //ECA_HPP