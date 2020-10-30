/**
 * @file restoration_plan_2.hpp
 * @author François Hamonic (francois.hamonic@gmail.com)
 * @brief Restorationplan2 class declaration
 * @version 0.1
 * @date 2020-10-26
 * 
 * @copyright Copyright (c) 2020
 */
#ifndef RESTORATION_PLAN_HPP
#define RESTORATION_PLAN_HPP

#include<map>
#include<list>
#include<memory>

#include"landscape/landscape.hpp"

/**
 * @brief A class for respresenting a restoration plan.
 * 
 * This class is essentially a sparse matrix implementation with both row major and column major structures to ensure linear time iteration. 
 */
class RestorationPlan {
    public:
        typedef int Option;
    private:
        const Landscape & _landscape;

        Graph_t::NodeMap<std::map<Option, double>> _nodeMap;
        Graph_t::ArcMap<std::map<Option, double>> _arcMap;

        std::vector<std::map<Graph_t::Node, int>> _options_nodes_idsMap;
        std::vector<std::map<Graph_t::Arc, int>> _options_arcs_idsMap;

        std::vector<std::vector<std::pair<Graph_t::Node, double>>> _options_nodes;
        std::vector<std::vector<std::pair<Graph_t::Arc, double>>> _options_arcs;

        std::vector<double> _costs;
    public:
        RestorationPlan(const Landscape & l) : _landscape(l), _nodeMap(l.getNetwork()), _arcMap(l.getNetwork()) {  }
        ~RestorationPlan() {}
            
        const Landscape & getLandscape() const { return _landscape; }

        /**
         * @brief Get the number of nodes concerned by option **i**
         * @param i - Option
         * @return int
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$
         */
        int getNbNodes(Option i) const {
            assert(contains(i));
            return _options_nodes[i].size();
        }

        /**
         * @brief Get the number of arcs concerned by option **i**
         * @param i - Option
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$
         */
        int getNbArcs(Option i) const {
            assert(contains(i));
            return _options_arcs[i].size();
        }

        /**
         * @brief Get the number of nodes and arcs concerned by option **i**
         * @param i - Option
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$
         */
        int getNbElements(Option i) const {
            assert(contains(i));
            return getNbNodes(i) + getNbArcs(i);
        }

        /**
         * @brief Return true if the option **i** concerns node **v**
         * @param i - Option
         * @param v - Node
         * @return bool true if the option **i** concerns node **v** false otherwise
         * @time \f$O(\log \#nodes(i))\f$
         * @space \f$O(1)\f$
         */
        bool contains(Option i, Graph_t::Node v) const {
            assert(contains(i));
            return _options_nodes_idsMap[i].contains(v);
        }

        /**
         * @brief Return true if the option **i** concerns arc **a**
         * @param i - Option
         * @param a - Arc
         * @return bool true if the option **i** concerns arc **a** false otherwise
         * @time \f$O(\log \#arcs(i))\f$
         * @space \f$O(1)\f$
         */
        bool contains(Option i, Graph_t::Arc a) const { 
            assert(contains(i)); 
            return _options_arcs_idsMap[i].contains(a);
        }

        /**
         * @brief Test if there is an option concerning the node **v**
         * @param v - Node
         * @return bool true if the option **i** concerns node **v** false otherwise
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$
         */
        bool contains(Graph_t::Node v) const { return _nodeMap[v].size() > 0; }

        /**
         * @brief Test if there is an option concerning the arc **a**
         * @param a - Arc
         * @return bool true if the option **i** concerns arc **a** false otherwise
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$
         */
        bool contains(Graph_t::Arc a) const { return _arcMap[a].size() > 0; }

        /**
         * @brief Add the node **v** to the option **i**
         * @param i - Option
         * @param v - Node
         * @param quality_gain - quality gain on **v**
         * @time \f$O(\log \#nodes(i) + \log \#options(v))\f$
         * @space \f$O(1)\f$ 
         */
        void addNode(Option i, Graph_t::Node v, double quality_gain) {
            assert(contains(i));
            if(contains(i, v)) {
                setQualityGain(i, v, getQualityGain(i,v) + quality_gain);
                return;
            }
            _options_nodes[i].push_back(std::pair<Graph_t::Node, double>(v, quality_gain));
            _options_nodes_idsMap[i][v] = _options_nodes[i].size()-1;
            _nodeMap[v][i] = quality_gain;
        }

        /**
         * @brief Add the arc **a** to the option **i**
         * @param i - Option
         * @param a - Arc 
         * @param restored_probability - restored probability of **a**
         * @time \f$O(\log \#arcs(i) + \log \#options(a))\f$
         * @space \f$O(1)\f$ 
         */
        void addArc(Option i, Graph_t::Arc a, double restored_probability) {
            assert(contains(i)); 
            if(contains(i, a)) {
                setRestoredProbability(i, a, std::max(getRestoredProbability(i,a), restored_probability));
                return;
            }
            _options_arcs[i].push_back(std::pair<Graph_t::Arc, double>(a, restored_probability));
            _options_arcs_idsMap[i][a] = _options_arcs[i].size()-1;
            _arcMap[a][i] = restored_probability;
        }

        /**
         * @brief Remove the node **v** from the option **i**
         * @param i - Option
         * @param v - Node
         * @time \f$O(\log \#nodes(i) + \log \#options(v))\f$
         * @space \f$O(1)\f$ 
         */
        void removeNode(Option i, Graph_t::Node v) {
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
         * @param i - Option
         * @param a - Arc
         * @time \f$O(\log \#arcs(i) + \log \#options(a))\f$
         * @space \f$O(1)\f$ 
         */
        void removeArc(Option i, Graph_t::Arc a) {
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
         * @param v - Node
         * @time \f$O(\sum_{i \in options(v)} \log \#nodes(i))\f$
         * @space \f$O(1)\f$ 
         */
        void removeNode(Graph_t::Node v) {
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
         * @param a - Arc
         * @time \f$O(\sum_{i \in options(a)} \log \#arcs(i))\f$
         * @space \f$O(1)\f$ 
         */
        void removeArc(Graph_t::Arc a) {
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
         * @param i - Option
         * @param v - Node
         * @return double 
         * @time \f$O(\log \#options(v))\f$
         * @space \f$O(1)\f$ 
         */
        double getQualityGain(Option i, Graph_t::Node v) const { 
            assert(contains(i, v)); 
            return _nodeMap[v].at(i);
        }

        /**
         * @brief Get the RestoredProbability of arc **a** in the option **i**
         * @param i - Option
         * @param a - Arc
         * @return double 
         * @time \f$O(\log \#options(a))\f$
         * @space \f$O(1)\f$ 
         */
        double getRestoredProbability(Option i, Graph_t::Arc a) const {
            assert(contains(i, a)); 
            return _arcMap[a].at(i);
        }

        /**
         * @brief Set the QualityGain of node **v** in the option **i**
         * @param i - Option
         * @param v - Node
         * @param quality_gain - quality gain on **v**
         * @time \f$O(\log \#options(v))\f$
         * @space \f$O(1)\f$ 
         */
        void setQualityGain(Option i, Graph_t::Node v, double quality_gain) {
            if(!contains(i, v)) return;
            const int id = _options_nodes_idsMap[i][v];
            _options_nodes[i][id].second = quality_gain;
            _nodeMap[v][i] = quality_gain;

        }

        /**
         * @brief Moves the quality gains of **from** to **to** in every options scaled by **scale**
         * @param from - Node
         * @param to - Node
         * @param scale - double
         * @time \f$O(\sum_{i \in options(a)} \log \#nodes(i))\f$
         * @space \f$O(1)\f$ 
         */
        void moveQualityGains(Graph_t::Node from, Graph_t::Node to, double scale=1.0) {
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
         * @param i - Option
         * @param a - Arc
         * @param restored_probability - restored probability of **a**
         * @time \f$O(\log \#options(a))\f$
         * @space \f$O(1)\f$ 
         */
        void setRestoredProbability(Option i, Graph_t::Arc a, double restored_probability) {
            if(!contains(i, a)) return;
            const int id = _options_arcs_idsMap[i][a];
            _options_arcs[i][id].second = restored_probability;
            _arcMap[a][i] = restored_probability;
        }

        /**
         * @brief Scale the restored probabilities of **a** in every options by **probability_scale**
         * @param a - Arc
         * @param probability_scale - double
         * @time \f$O(\sum_{i \in options(a)} \log \#arcs(i))\f$
         * @space \f$O(1)\f$ 
         */
        void updateProbability(Graph_t::Arc a, double probability_scale) {
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
         * @param i - Option
         * @param v - Node
         * @return double 
         * @time \f$O(\log \#nodes(i))\f$
         * @space \f$O(1)\f$ 
         */
        double id(Option i, Graph_t::Node v) const { return _options_nodes_idsMap.at(i).at(v); }

        /**
         * @brief  Get the id of the arc **a** in the option **i**
         * @param i - Option
         * @param a - Arc
         * @return double 
         * @time \f$O(\log \#arcs(i))\f$
         * @space \f$O(1)\f$ 
         */
        double id(Option i, Graph_t::Arc a) const { 
            return _options_arcs_idsMap.at(i).at(a);
        }

        /**
         * @brief add an option of cost **c** and returns its id
         * @param c - cost
         * @return Option 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        Option addOption(double c) {
            _options_nodes_idsMap.push_back(std::map<Graph_t::Node, int>());
            _options_arcs_idsMap.push_back(std::map<Graph_t::Arc, int>());
            _options_nodes.push_back(std::vector<std::pair<Graph_t::Node, double>>());
            _options_arcs.push_back(std::vector<std::pair<Graph_t::Arc, double>>());
            _costs.push_back(c);
            return _costs.size()-1;
        }

        /**
         * @brief Get the number of options
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        int getNbOptions() const { return _costs.size(); }

        /**
         * @brief Return true if the restoration plan contains an option of id **i**
         * @param i - Option
         * @return bool 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        bool contains(Option i) const { return i>=0 && i<getNbOptions(); }

        /**
         * @brief Set the cost of option **i**
         * @param i - Option
         * @param c -  cost
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        void setCost(Option i, double cost) { _costs[i] = cost; }

        /**
         * @brief Get the cost of option **i**
         * @param i - Option
         * @return double 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        double getCost(Option i) const { return _costs[i]; }

        /**
         * @brief Test if option **i** is empty 
         * @param i - Option
         * @return true if option **i** is empty
         * @return false otherwise
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        bool isEmpty(Option i) const { return _options_nodes[i].size()>0 || _options_arcs[i].size()>0; }

        /**
         * @brief Returns the map of options concerning the node **v**
         * @param v - Node
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        const std::map<Option, double> & getOptions(Graph_t::Node v) const { return _nodeMap[v]; }

        /**
         * @brief Returns the map of options concerning the arc **a**
         * @param a - Arc
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        const std::map<Option, double> & getOptions(Graph_t::Arc a) const { return _arcMap[a]; }

        /**
         * @brief Returns the map of options concerning the node **v**
         * @param v - Node
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        const std::vector<std::pair<Graph_t::Node, double>> & getNodes(Option i) const { return _options_nodes[i]; }

        /**
         * @brief Returns the map of options concerning the arc **a**
         * @param a - Arc
         * @return int 
         * @time \f$O(1)\f$
         * @space \f$O(1)\f$ 
         */
        const std::vector<std::pair<Graph_t::Arc, double>> & getArcs(Option i) const { return _options_arcs[i]; }

        /**
         * @brief Erase invalid nodes from the restoration plan
         * @time \f$O(\sum_{i \in options} \#nodes(i))\f$
         * @space \f$O(\#invalid\_nodes)\f$ 
         */
        void eraseInvalidNodes() {
            const Graph_t & graph = _landscape.getNetwork();
            for(Option i=0; i<getNbOptions(); ++i) {
                std::vector<int> free_ids;
                std::map<lemon::ListDigraph::Node, int> & m = _options_nodes_idsMap[i];
                for(auto it = m.cbegin(), next_it = it; it != m.cend(); it = next_it) {
                    ++next_it;
                    Graph_t::Node v = it->first;
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
        void eraseInvalidArcs() {
            const Graph_t & graph = _landscape.getNetwork();
            for(Option i=0; i<getNbOptions(); ++i) {
                std::vector<int> free_ids;
                std::map<lemon::ListDigraph::Arc, int> & m = _options_arcs_idsMap[i];
                for(auto it = m.cbegin(), next_it = it; it != m.cend(); it = next_it) {
                    ++next_it;
                    Graph_t::Arc a = it->first;
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
        void eraseInvalidElements() {
            eraseInvalidNodes();
            eraseInvalidArcs();
        }
};

std::ostream & operator<<(std::ostream & in, const RestorationPlan & plan);

#endif //RESTORATION_PLAN_2_HPP