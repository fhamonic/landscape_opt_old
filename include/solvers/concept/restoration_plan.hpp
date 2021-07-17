/**
 * @file restoration_plan.hpp
 * @author François Hamonic (francois.hamonic@gmail.com)
 * @brief Restorationplan class declaration
 * @version 0.2
 * @date 2020-10-26
 * 
 * @copyright Copyright (c) 2020
 */
#ifndef RESTORATION_PLAN_HPP
#define RESTORATION_PLAN_HPP

#include<cassert>
#include<map>
#include<list>
#include<memory>
#include <numeric>

#include"landscape/concept/abstract_landscape.hpp"

/**
 * @brief A class for respresenting a restoration plan.
 * 
 * This class is essentially a sparse matrix implementation with both row major and column major structures to ensure linear time iteration. 
 */
template <typename LS> //requires concepts::IsLandscape<LS> //c++20
class RestorationPlan{
    public:
        using Graph = typename LS::Graph;
        using Node = typename LS::Node;
        using Arc = typename LS::Arc;

        using Option = int;
    private:
        const LS & _landscape;

        typename Graph::template NodeMap<std::map<Option, double>> _nodeMap;
        typename Graph::template ArcMap<std::map<Option, double>> _arcMap;

        std::vector<std::map<Node, int>> _options_nodes_idsMap;
        std::vector<std::map<Arc, int>> _options_arcs_idsMap;

        std::vector<std::vector<std::pair<Node, double>>> _options_nodes;
        std::vector<std::vector<std::pair<Arc, double>>> _options_arcs;

        std::vector<double> _costs;
    public:
        RestorationPlan(const LS & l) : _landscape(l), _nodeMap(l.getNetwork()), _arcMap(l.getNetwork()) {  }
        RestorationPlan(const RestorationPlan<LS> & rp) : _landscape(rp.getLandscape()), _nodeMap(_landscape.getNetwork()), _arcMap(_landscape.getNetwork()) { assert(false && "No fucking copy constructor"); }
        RestorationPlan(RestorationPlan<LS>&& rp) : _landscape(rp.getLandscape()), _nodeMap(_landscape.getNetwork()), _arcMap(_landscape.getNetwork()) { assert(false && "No fucking move constructor"); }
        ~RestorationPlan() {}
        RestorationPlan<LS> & operator=(const RestorationPlan<LS>&) { assert(false && "Fuck"); return *this; };
        RestorationPlan<LS> & operator=(RestorationPlan<LS>&&) { assert(false && "Fuck"); return *this; };
            
        const LS & getLandscape() const noexcept { return _landscape; }

        /**
         * @brief Get the number of nodes concerned by option **i**
         * @param i Option
         * @return int
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$
         */
        int getNbNodes(Option i) const noexcept {
            assert(contains(i));
            return _options_nodes[i].size();
        }

        /**
         * @brief Get the number of nodes concerned by at least one option
         * @return int
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$
         */
        int getNbNodes() const noexcept {
            return _nodeMap.size();
        }

        /**
         * @brief Get the number of arcs concerned by option **i**
         * @param i Option
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$
         */
        int getNbArcs(Option i) const noexcept {
            assert(contains(i));
            return _options_arcs[i].size();
        }

        /**
         * @brief Get the number of arcs concerned by at least one option
         * @return int
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$
         */
        int getNbArcs() const noexcept {
            typename Graph::template ArcMap<bool> seen_arcs(_landscape.getNetwork(), false);
            int count = 0;
            for(const auto & arc_option : _options_arcs) {
                for(const auto & [a, v] : arc_option) {
                    if(seen_arcs[a]) continue;
                    ++count;
                    seen_arcs[a] = true;
                }
            }
            return count;
        }

        /**
         * @brief Get the number of nodes and arcs concerned by option **i**
         * @param i Option
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$
         */
        int getNbElements(Option i) const noexcept {
            assert(contains(i));
            return getNbNodes(i) + getNbArcs(i);
        }

        /**
         * @brief Get the number of nodes and arcs concerned by at least one option
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$
         */
        int getNbElements() const noexcept {
            return getNbNodes() + getNbArcs();
        }

        /**
         * @brief Return true if the option **i** concerns node **v**
         * @param i Option
         * @param v Node
         * @return bool true if the option **i** concerns node **v** false otherwise
         * @time \f$O(\log \#nodes(i))\f$
         * @space \f$O(1)\f$
         */
        bool contains(Option i, Node v) const noexcept {
            assert(contains(i));
            return _options_nodes_idsMap[i].find(v) != _options_nodes_idsMap[i].end();
        }

        /**
         * @brief Return true if the option **i** concerns arc **a**
         * @param i Option
         * @param a Arc
         * @return bool true if the option **i** concerns arc **a** false otherwise
         * @time \f$O(\log \#arcs(i))\f$
         * @space \f$O(1)\f$
         */
        bool contains(Option i, Arc a) const noexcept { 
            assert(contains(i)); 
            return _options_arcs_idsMap[i].find(a) != _options_arcs_idsMap[i].end();
        }

        /**
         * @brief Test if there is an option concerning the node **v**
         * @param v Node
         * @return bool true if the option **i** concerns node **v** false otherwise
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$
         */
        bool contains(Node v) const noexcept { return _nodeMap[v].size() > 0; }

        /**
         * @brief Test if there is an option concerning the arc **a**
         * @param a Arc
         * @return bool true if the option **i** concerns arc **a** false otherwise
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$
         */
        bool contains(Arc a) const noexcept { return _arcMap[a].size() > 0; }

        /**
         * @brief Add the node **v** to the option **i**
         * @param i Option
         * @param v Node
         * @param quality_gain - quality gain on **v**
         * @time \f$O(\log \#nodes(i) + \log \#options(v))\f$
         * @space \f$O(1)\f$ 
         */
        void addNode(Option i, Node v, double quality_gain) noexcept {
            assert(contains(i));
            if(contains(i, v)) {
                setQualityGain(i, v, getQualityGain(i,v) + quality_gain);
                return;
            }
            _options_nodes[i].emplace_back(v, quality_gain);
            _options_nodes_idsMap[i][v] = _options_nodes[i].size()-1;
            _nodeMap[v][i] = quality_gain;
        }

        /**
         * @brief Add the arc **a** to the option **i**
         * @param i Option
         * @param a Arc 
         * @param restored_probability - restored probability of **a**
         * @time \f$O(\log \#arcs(i) + \log \#options(a))\f$
         * @space \f$O(1)\f$ 
         */
        void addArc(Option i, Arc a, double restored_probability) noexcept {
            assert(contains(i)); 
            if(contains(i, a)) {
                setRestoredProbability(i, a, std::max(getRestoredProbability(i,a), restored_probability));
                return;
            }
            _options_arcs[i].emplace_back(a, restored_probability);
            _options_arcs_idsMap[i][a] = _options_arcs[i].size()-1;
            _arcMap[a][i] = restored_probability;
        }

        /**
         * @brief Remove the node **v** from the option **i**
         * @param i Option
         * @param v Node
         * @time \f$O(\log \#nodes(i) + \log \#options(v))\f$
         * @space \f$O(1)\f$ 
         */
        void removeNode(Option i, Node v) noexcept {
            assert(contains(i)); 
            if(!contains(i, v)) return;
            const int id = _options_nodes_idsMap[i][v];
            const int last_id = _options_nodes[i].size()-1;
            if(id < last_id) {
                std::swap(_options_nodes[i][id], _options_nodes[i][last_id]);
                _options_nodes_idsMap[i][_options_nodes[i][id].first] = id;
            }
            _options_nodes[i].pop_back();
            _options_nodes_idsMap[i].erase(v);
            _nodeMap[v].erase(i);  
        }

        /**
         * @brief Remove the arc **a** from the option **i**
         * @param i Option
         * @param a Arc
         * @time \f$O(\log \#arcs(i) + \log \#options(a))\f$
         * @space \f$O(1)\f$ 
         */
        void removeArc(Option i, Arc a) noexcept {
            assert(contains(i));
            if(!contains(i, a)) return;
            const int id = _options_arcs_idsMap[i][a];
            const int last_id = _options_arcs[i].size()-1;
            if(id < last_id) {
                std::swap(_options_arcs[i][id], _options_arcs[i][last_id]);
                _options_arcs_idsMap[i][_options_arcs[i][id].first] = id;
            }
            _options_arcs[i].pop_back();
            _options_arcs_idsMap[i].erase(a);
            _arcMap[a].erase(i);
        }

        /**
         * @brief Remove the node **v** from every option
         * @param v Node
         * @time \f$O(\sum_{i \in options(v)} \log \#nodes(i))\f$
         * @space \f$O(1)\f$ 
         */
        void removeNode(Node v) noexcept {
            if(!contains(v)) return;
            for(auto const& [i, val] : _nodeMap[v]) {
                const int id = _options_nodes_idsMap[i][v];
                const int last_id = _options_nodes[i].size()-1;
                if(id < last_id) {
                    std::swap(_options_nodes[i][id], _options_nodes[i][last_id]);
                    _options_nodes_idsMap[i][_options_nodes[i][id].first] = id;
                }
                _options_nodes[i].pop_back();
                _options_nodes_idsMap[i].erase(v);
            }
            _nodeMap[v].clear();
        }

        /**
         * @brief Remove the arc **a** from every option
         * @param a Arc
         * @time \f$O(\sum_{i \in options(a)} \log \#arcs(i))\f$
         * @space \f$O(1)\f$ 
         */
        void removeArc(Arc a) noexcept {
            if(!contains(a)) return;
            for(auto const& [i, val] : _arcMap[a]) {
                const int id = _options_arcs_idsMap[i][a];
                const int last_id = _options_arcs[i].size()-1;
                if(id < last_id) {
                    std::swap(_options_arcs[i][id], _options_arcs[i][last_id]);
                    _options_arcs_idsMap[i][_options_arcs[i][id].first] = id;
                }
                _options_arcs[i].pop_back();
                _options_arcs_idsMap[i].erase(a);
            }
            _arcMap[a].clear();
        }

        /**
         * @brief Get the QualityGain of node **v** in the option **i**
         * @param i Option
         * @param v Node
         * @return double 
         * @time \f$O(\log \#options(v))\f$
         * @space \f$O(1)\f$ 
         */
        double getQualityGain(Option i, Node v) const noexcept { 
            assert(contains(i, v)); 
            return _nodeMap[v].at(i);
        }

        /**
         * @brief Get the RestoredProbability of arc **a** in the option **i**
         * @param i Option
         * @param a Arc
         * @return double 
         * @time \f$O(\log \#options(a))\f$
         * @space \f$O(1)\f$ 
         */
        double getRestoredProbability(Option i, Arc a) const noexcept {
            assert(contains(i, a)); 
            return _arcMap[a].at(i);
        }

        /**
         * @brief Set the QualityGain of node **v** in the option **i**
         * @param i Option
         * @param v Node
         * @param quality_gain - quality gain on **v**
         * @time \f$O(\log \#options(v))\f$
         * @space \f$O(1)\f$ 
         */
        void setQualityGain(Option i, Node v, double quality_gain) noexcept {
            if(!contains(i, v)) return;
            const int id = _options_nodes_idsMap[i][v];
            _options_nodes[i][id].second = quality_gain;
            _nodeMap[v][i] = quality_gain;

        }

        /**
         * @brief Moves the quality gains of **from** to **to** in every options scaled by **scale**
         * @param from Node
         * @param to Node
         * @param scale double
         * @time \f$O(\sum_{i \in options(a)} \log \#nodes(i))\f$
         * @space \f$O(1)\f$ 
         */
        void moveQualityGains(Node from, Node to, double scale=1.0) noexcept {
            if(!contains(from)) return;            
            std::map<Option, double> & from_m = _nodeMap[from];
            for(auto it = from_m.begin(); it != from_m.end(); ++it) {
                const Option i = it->first;
                const double quality_gain = it->second;
                addNode(i, to, scale * quality_gain);
            }
            removeNode(from);
        }

        /**
         * @brief Set the RestoredProbability of arc **a** in the option **i**
         * @param i Option
         * @param a Arc
         * @param restored_probability - restored probability of **a**
         * @time \f$O(\log \#options(a))\f$
         * @space \f$O(1)\f$ 
         */
        void setRestoredProbability(Option i, Arc a, double restored_probability) noexcept {
            if(!contains(i, a)) return;
            const int id = _options_arcs_idsMap[i][a];
            _options_arcs[i][id].second = restored_probability;
            _arcMap[a][i] = restored_probability;
        }

        /**
         * @brief Scale the restored probabilities of **a** in every options by **probability_scale**
         * @param a Arc
         * @param probability_scale - double
         * @time \f$O(\sum_{i \in options(a)} \log \#arcs(i))\f$
         * @space \f$O(1)\f$ 
         */
        void updateProbability(Arc a, double probability_scale) noexcept {
            if(!contains(a)) return;
            std::map<Option, double> & m = _arcMap[a];
            for(auto it = m.begin(); it != m.end(); ++it) {
                const Option i = it->first;
                const int id = _options_arcs_idsMap[i][a];
                _options_arcs[i][id].second *= probability_scale;
                it->second *= probability_scale;
            }
        }

        /**
         * @brief  Get the id of the node **v** in the option **i**
         * @param i Option
         * @param v Node
         * @return double 
         * @time \f$O(\log \#nodes(i))\f$
         * @space \f$O(1)\f$ 
         */
        double id(Option i, Node v) const noexcept { return _options_nodes_idsMap.at(i).at(v); }

        /**
         * @brief  Get the id of the arc **a** in the option **i**
         * @param i Option
         * @param a Arc
         * @return double 
         * @time \f$O(\log \#arcs(i))\f$
         * @space \f$O(1)\f$ 
         */
        double id(Option i, Arc a) const noexcept { 
            return _options_arcs_idsMap.at(i).at(a);
        }

        /**
         * @brief add an option of cost **c** and returns its id
         * @param c cost
         * @return Option 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        Option addOption(double c) noexcept {
            _options_nodes_idsMap.emplace_back();
            _options_arcs_idsMap.emplace_back();
            _options_nodes.emplace_back();
            _options_arcs.emplace_back();
            _costs.push_back(c);
            return _costs.size()-1;
        }

        /**
         * @brief Get the number of options
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        int getNbOptions() const noexcept { return _costs.size(); }

        /**
         * @brief Return true if the restoration plan contains an option of id **i**
         * @param i - Option
         * @return bool 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        bool contains(Option i) const noexcept { return i>=0 && i<getNbOptions(); }

        /**
         * @brief Set the cost of option **i**
         * @param i Option
         * @param c cost
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        void setCost(Option i, double cost) noexcept { _costs[i] = cost; }

        /**
         * @brief Get the cost of option **i**
         * @param i Option
         * @return double 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        double getCost(Option i) const noexcept { return _costs[i]; }

        /**
         * @brief Test if option **i** is empty 
         * @param i Option
         * @return true if option **i** is empty
         * @return false otherwise
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        bool isEmpty(Option i) const noexcept { return _options_nodes[i].size()>0 || _options_arcs[i].size()>0; }

        /**
         * @brief Returns the map of options concerning the node **v**
         * @param v Node
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        const std::map<Option, double> & getOptions(Node v) const noexcept { return _nodeMap[v]; }

        /**
         * @brief Returns the map of options concerning the arc **a**
         * @param a Arc
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        const std::map<Option, double> & getOptions(Arc a) const noexcept { return _arcMap[a]; }

        /**
         * @brief Returns the map of options concerning the node **v**
         * @param v Node
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        const std::vector<std::pair<Node, double>> & getNodes(Option i) const noexcept { return _options_nodes[i]; }

        /**
         * @brief Returns the map of options concerning the arc **a**
         * @param a Arc
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        const std::vector<std::pair<Arc, double>> & getArcs(Option i) const noexcept { return _options_arcs[i]; }

        /**
         * @brief Erase invalid nodes from the restoration plan
         * @time \f$O(\sum_{i \in options} \#nodes(i))\f$
         * @space \f$O(\#invalid\_nodes)\f$ 
         */
        void eraseInvalidNodes() noexcept {
            const Graph & graph = _landscape.getNetwork();
            for(Option i=0; i<getNbOptions(); ++i) {
                std::vector<int> free_ids;
                std::map<Node, int> & m = _options_nodes_idsMap[i];
                for(auto it = m.cbegin(), next_it = it; it != m.cend(); it = next_it) {
                    ++next_it;
                    Node v = it->first;
                    const int id = it->second;
                    if(graph.valid(v)) continue;
                    free_ids.push_back(id);             
                    m.erase(it);
                }
                auto it_free_id = free_ids.begin();
                for(auto it = m.begin(); it != m.end(); ++it) {
                    const int id = it->second;
                    if(id < static_cast<int>(_options_nodes[i].size()) - static_cast<int>(free_ids.size())) continue;
                    _options_nodes[i][*it_free_id] = _options_nodes[i][id];
                    it->second = *it_free_id;
                    ++it_free_id;
                }
                for(int j=0; j<static_cast<int>(free_ids.size()); ++j)
                    _options_nodes[i].pop_back();
            } 
        }

        /**
         * @brief Erase invalid nodes from the restoration plan
         * @time \f$O(\sum_{i \in options} \#arcs(i))\f$
         * @space \f$O(\#invalid\_arcs)\f$ 
         */
        void eraseInvalidArcs() noexcept {
            const Graph & graph = _landscape.getNetwork();
            for(Option i=0; i<getNbOptions(); ++i) {
                std::vector<int> free_ids;
                std::map<Arc, int> & m = _options_arcs_idsMap[i];
                for(auto it = m.cbegin(), next_it = it; it != m.cend(); it = next_it) {
                    ++next_it;
                    Arc a = it->first;
                    const int id = it->second;
                    if(graph.valid(a)) continue;
                    free_ids.push_back(id);             
                    m.erase(it);
                }
                auto it_free_id = free_ids.begin();
                for(auto it = m.begin(); it != m.end(); ++it) {
                    const int id = it->second;
                    if(id < static_cast<int>(_options_arcs[i].size()) - static_cast<int>(free_ids.size())) continue;
                    _options_arcs[i][*it_free_id] = _options_arcs[i][id];
                    it->second = *it_free_id;
                    ++it_free_id;
                }
                for(int j=0; j<static_cast<int>(free_ids.size()); ++j)
                    _options_arcs[i].pop_back();
            } 
        }

        /**
         * @brief Erase invalid nodes and arcs from the restoration plan
         * @time \f$O(\sum_{i \in options} \#nodes(i) + \#arcs(i))\f$
         * @space \f$O(\#invalid\_nodes + \#invalid\_arcs)\f$ 
         */
        void eraseInvalidElements() noexcept {
            eraseInvalidNodes();
            eraseInvalidArcs();
        }

        /**
         * @brief Comptes the total cost of the restoration plan, i.e. sum of options costs
         * @return double 
         */
        double totalCost() const noexcept { return std::accumulate(_costs.begin(), _costs.end(), 0.0); }
};

template <typename LS> //requires concepts::IsLandscape<LS> //c++20
std::ostream & operator<<(std::ostream & in, const RestorationPlan<LS> & plan);

#endif //RESTORATION_PLAN_2_HPP