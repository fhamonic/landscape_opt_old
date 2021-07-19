#ifndef PARALLEL_ECA_HPP
#define PARALLEL_ECA_HPP

#include <algorithm>
#include <execution>

#include "indices/concept/connectivity_index.hpp"

#include "algorithms/multiplicative_dijkstra.h"

#include "lemon/bits/map_extender.h"

#include "solvers/concept/solution.hpp"
#include "landscape/decored_landscape.hpp"

class Parallel_ECA : public concepts::ConnectivityIndex {
    private:
        static Parallel_ECA singleton;
        Parallel_ECA();
    public:
        static Parallel_ECA & get() noexcept {
            return singleton;
        }
        ~Parallel_ECA();
   
        static double P_func(const double d, const double alpha) {
            assert(d >= 0);
            return std::exp(-d/alpha);
        }

        static double inv_P_func(const double p, const double alpha) {
            assert(p >= 0 && p <= 1);
            return -alpha * std::log(p);
        }

        template <typename LS>
        double eval_solution(const LS & landscape, const RestorationPlan<LS>& plan, const Solution & solution) const {
            DecoredLandscape<LS> decored_landscape(landscape);
            for(typename RestorationPlan<LS>::Option i=0; i<plan.getNbOptions(); ++i)
                decored_landscape.apply(plan, i, solution.getCoef(i));
            double value = eval(decored_landscape);
            return value;
        }

        /**
         * @brief Computes the value of the Parallel_ECA index of the specified landscape.
         * 
         * @time \f$O(n \cdot (m + n) \log n)\f$ where \f$n\f$ is the number of nodes and \f$m\f$ the number of arcs
         * @space \f$O(m)\f$ where \f$m\f$ is the number of arcs
         */
        template <typename GR, typename QM, typename PM, typename CM>
        double eval(const concepts::AbstractLandscape<GR, QM, PM, CM> & landscape, const typename GR::template NodeMap<bool> & nodeFilter) {
            const GR & g = landscape.getNetwork();
            const QM & qualityMap = landscape.getQualityMap();
            const PM & probabilityMap = landscape.getProbabilityMap();
            
            std::vector<typename GR::Node> nodes;
            for(typename GR::NodeIt s(g); s != lemon::INVALID; ++s) {
                if(!nodeFilter[s])
                    continue;
                nodes.push_back(s);
            }

            return std::sqrt(std::transform_reduce(std::execution::par_unseq, nodes.begin(), nodes.end(), 0.0, std::plus<>(), [&](typename GR::Node s){
                double sum = 0;
                lemon::MultiplicativeSimplerDijkstra<GR, PM> dijkstra(g, probabilityMap);
                dijkstra.init(s);
                while(!dijkstra.emptyQueue()) {
                    std::pair<typename GR::Node, double> pair = dijkstra.processNextNode();
                    typename GR::Node t = pair.first;
                    if(!nodeFilter[t])
                        continue;
                    const double p_st = pair.second;
                    sum += qualityMap[s] * qualityMap[t] * p_st;
                }
                return sum;
            }));
        }

        /**
         * @brief Computes the value of the Parallel_ECA index of the specified landscape.
         * 
         * @time \f$O(n \cdot (m + n) \log n)\f$ where \f$n\f$ is the number of nodes and \f$m\f$ the number of arcs
         * @space \f$O(m)\f$ running and \f$O(1)\f$ returning where \f$m\f$ is the number of arcs
         */
        template <typename GR, typename QM, typename PM, typename CM>
        double eval(const concepts::AbstractLandscape<GR, QM, PM, CM> & landscape) {
            const GR & g = landscape.getNetwork();
            const QM & qualityMap = landscape.getQualityMap();
            const PM & probabilityMap = landscape.getProbabilityMap();
            
            std::vector<typename GR::Node> nodes;
            for(typename GR::NodeIt s(g); s != lemon::INVALID; ++s)
                nodes.push_back(s);

            return std::sqrt(std::transform_reduce(std::execution::par_unseq, nodes.begin(), nodes.end(), 0.0, std::plus<>(), [&](typename GR::Node s){
                double sum = 0;
                lemon::MultiplicativeSimplerDijkstra<GR, PM> dijkstra(g, probabilityMap);
                dijkstra.init(s);
                while(!dijkstra.emptyQueue()) {
                    std::pair<typename GR::Node, double> pair = dijkstra.processNextNode();
                    typename GR::Node t = pair.first;
                    const double p_st = pair.second;
                    sum += qualityMap[s] * qualityMap[t] * p_st;
                }
                return sum;
            }));
        }
};

#endif //Parallel_ECA_HPP