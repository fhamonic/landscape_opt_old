/**
 * @file restoration_plan_2.hpp
 * @author François Hamonic (francois.hamonic@gmail.com)
 * @brief Restorationplan2 class declaration
 * @version 0.1
 * @date 2020-10-26
 * 
 * @copyright Copyright (c) 2020
 */
#ifndef RESTORATION_PLAN_2_HPP
#define RESTORATION_PLAN_2_HPP

#include<map>
#include<list>
#include<memory>

#include"landscape/landscape.hpp"

/**
 * @brief A class for respresenting a restoration plan.
 * 
 * This class is essentially a sparse matrix implementation with both row major and column major structures to ensure linear time iteration. 
 */
class RestorationPlan2 {
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
        RestorationPlan2(const Landscape & l) : _landscape(l), _nodeMap(l.getNetwork()), _arcMap(l.getNetwork()) {  }
    ~RestorationPlan2() {  }
            
    const Landscape & getLandscape() const { return _landscape; }

    /**
     * @brief Get the number of nodes concerned by option \f$i\f$
     * @param i 
     * @return int
     * @time \f$ O(\log #options) \f$
     * @space \f$ O(1) \f$
     */
    int getNbNodes(Option i) const { return _options_nodes[i].size(); }
    /**
     * @brief Get the number of arcs concerned by option \f$i\f$
     * @param i 
     * @return int 
     * @time \f$ O(1) \f$
     * @space \f$ O(1) \f$
     */
    int getNbArcs(Option i) const { return _options_arcs[i].size(); }
    /**
     * @brief Return true if the option \f$i\f$ concerns node \f$v\f$
     * @param i 
     * @param v 
     * @return bool true if the option \f$i\f$ concerns node \f$v\f$ false otherwise
     * @time \f$ O(\log #nodes(i)) \f$
     * @space \f$ O(1) \f$
     */
    bool contains(Option i, Graph_t::Node v) const { return _options_nodes_idsMap[i].contains(v); }
    /**
     * @brief Return true if the option \f$i\f$ concerns arc \f$a\f$
     * @param i 
     * @param a
     * @return bool true if the option \f$i\f$ concerns arc \f$a\f$ false otherwise
     * @time \f$ O(\log #arcs(i)) \f$
     * @space \f$ O(1) \f$
     */
    bool contains(Option i, Graph_t::Arc a) const { return _options_arcs_idsMap[i].contains(a); }

    /**
     * @brief Test if there is an option concerning the node \f$v\f$
     * @param v
     * @return bool true if the option \f$i\f$ concerns node \f$v\f$ false otherwise
     * @time \f$ O(1) \f$
     * @space \f$ O(1) \f$
     */
    bool contains(Graph_t::Node v) const { return _nodeMap[v].size() > 0; }
    /**
     * @brief Test if there is an option concerning the arc \f$a\f$
     * @param a
     * @return bool true if the option \f$i\f$ concerns arc \f$a\f$ false otherwise
     * @time \f$ O(1) \f$
     * @space \f$ O(1) \f$
     */
    bool contains(Graph_t::Arc a) const { return _arcMap[a].size() > 0; }

    /**
     * @brief Add the node \f$v\f$ to the option \f$i\f$     * 
     * @param i 
     * @param v 
     * @param quality_gain 
     * @time \f$ O(\log #nodes(i) + \log #options(v)) \f$
     * @space \f$ O(1) \f$ 
     */
    void addNode(Option i, Graph_t::Node v, double quality_gain) {
        if(contains(i, v)) return;
        _options_nodes[i].push_back(std::pair<Graph_t::Node, double>(v, quality_gain));
        _options_nodes_idsMap[i][v] = _options_nodes[i].size()-1;
        _nodeMap[v][i] = quality_gain;
    }
    /**
     * @brief Add the arc \f$a\f$ to the option \f$i\f$     * 
     * @param i 
     * @param a 
     * @param restored_probability 
     * @time \f$ O(\log #arcs(i) + \log #options(a)) \f$
     * @space \f$ O(1) \f$ 
     */
    void addArc(Option i, Graph_t::Arc a, double restored_probability) {
        if(contains(i, a)) return;
        _options_arcs[i].push_back(std::pair<Graph_t::Arc, double>(a, restored_probability));
        _options_arcs_idsMap[i][a] = _options_arcs[i].size()-1;
        _arcMap[a][i] = restored_probability;
    }

    /**
     * @brief Remove the node \f$v\f$ from the option \f$i\f$ 
     * @param i 
     * @param v 
     * @time \f$ O(\log #nodes(i) + \log #options(v)) \f$
     * @space \f$ O(1) \f$ 
     */
    void removeNode(Option i, Graph_t::Node v) {
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
     * @brief Remove the arc \f$a\f$ from the option \f$i\f$ 
     * @param i 
     * @param a 
     * @time \f$ O(\log #arcs(i) + \log #options(a)) \f$
     * @space \f$ O(1) \f$ 
     */
    void removeArc(Option i, Graph_t::Arc a) {
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
     * @brief Remove the node \f$v\f$ from every option
     * @param v 
     * @time \f$ O(#options(v) \log #nodes(i)) \f$
     * @space \f$ O(1) \f$ 
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
     * @brief Remove the arc \f$a\f$ from every option
     * @param v 
     * @time \f$ O(#options(a) \log #arcs(i)) \f$
     * @space \f$ O(1) \f$ 
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
     * @brief Get the QualityGain of node \f$v\f$ in the option \f$i\f$
     * @param i 
     * @param v 
     * @return double 
     * @time \f$ O(\log #options(v)) \f$
     * @space \f$ O(1) \f$ 
     */
    double getQualityGain(Option i, Graph_t::Node v) const { return _nodeMap[v].at(i); }
    /**
     * @brief Get the RestoredProbability of arc \f$a\f$ in the option \f$i\f$
     * @param i 
     * @param a 
     * @return double 
     * @time \f$ O(\log #options(a)) \f$
     * @space \f$ O(1) \f$ 
     */
    double getRestoredProbability(Option i, Graph_t::Arc a) const { return _arcMap[a].at(i); }

    /**
     * @brief  Get the id of the node \f$v\f$ in the option \f$i\f$
     * @param i 
     * @param v 
     * @return double 
     * @time \f$ O(\log #nodes(i)) \f$
     * @space \f$ O(1) \f$ 
     */
    double id(Option i, Graph_t::Node v) const { return _options_nodes_idsMap[i].at(v); }
    /**
     * @brief  Get the id of the arc \f$a\f$ in the option \f$i\f$
     * @param i 
     * @param a 
     * @return double 
     * @time \f$ O(\log #arcs(i)) \f$
     * @space \f$ O(1) \f$ 
     */
    double id(Option i, Graph_t::Arc a) const { return _options_arcs_idsMap[i].at(a); }

    /**
     * @brief add an option of cost \f$c\f$ and returns its id
     * @param c 
     * @return Option 
     * @time \f$ O(1) \f$
     * @space \f$ O(1) \f$ 
     */
    Option addOption(double c) {
        _options_nodes.push_back(std::vector<std::pair<Graph_t::Node, double>>());
        _options_arcs.push_back(std::vector<std::pair<Graph_t::Arc, double>>());
        _costs.push_back(c);
        return _costs.size()-1;
    }
    /**
     * @brief Set the cost of option \f$i\f$
     * @param i 
     * @param cost 
     * @time \f$ O(1) \f$
     * @space \f$ O(1) \f$ 
     */
    void setCost(Option i, double cost) { _costs[i] = cost; }
    /**
     * @brief Get the cost of option \f$i\f$
     * @param i 
     * @return double 
     * @time \f$ O(1) \f$
     * @space \f$ O(1) \f$ 
     */
    double getCost(Option i) const { return _costs[i]; }

    /**
     * @brief Test if option \f$i\f$ is empty 
     * @param i 
     * @return true if option \f$i\f$ is empty
     * @return false otherwise
     * @time \f$ O(1) \f$
     * @space \f$ O(1) \f$ 
     */
    bool isEmpty(Option i) const { return _options_nodes[i].size()>0 || _options_arcs[i].size()>0; }
    /**
     * @brief Get the number of options
     * @return int 
     * @time \f$ O(1) \f$
     * @space \f$ O(1) \f$ 
     */
    int getNbOptions() const { return _costs.size(); }

    /**
     * @brief Returns the map of options concerning the node \f$v\f$
     * @param v 
     * @return int 
     * @time \f$ O(1) \f$
     * @space \f$ O(1) \f$ 
     */
    const std::map<Option, double> & getOptions(Graph_t::Node v) const { return _nodeMap[v]; }
    /**
     * @brief Returns the map of options concerning the arc \f$a\f$
     * @param a 
     * @return int 
     * @time \f$ O(1) \f$
     * @space \f$ O(1) \f$ 
     */
    const std::map<Option, double> & getOptions(Graph_t::Arc a) const { return _arcMap[a]; }

    void cleanInvalidNodes() {
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
                Graph_t::Node v = it->first;
                const int id = it->second;
                if(id < _options_nodes[i].size()-free_ids.size()) continue;
                _options_nodes[i][*it_free_id] = _options_nodes[i][id];
                it->second = *it_free_id;
                ++it_free_id;
            }
            for(int j=0; j<free_ids.size(); ++j)
                _options_nodes[i].pop_back();
        } 
    }

    void cleanInvalidArcs() {
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
                Graph_t::Arc a = it->first;
                const int id = it->second;
                if(id < _options_arcs[i].size()-free_ids.size()) continue;
                _options_arcs[i][*it_free_id] = _options_arcs[i][id];
                it->second = *it_free_id;
                ++it_free_id;
            }
            for(int j=0; j<free_ids.size(); ++j)
                _options_arcs[i].pop_back();
        } 
    }
    
    void cleanInvalidElements() {
        cleanInvalidNodes();
        cleanInvalidArcs();
    }
};

std::ostream & operator<<(std::ostream & in, const RestorationPlan2 & plan);

#endif //RESTORATION_PLAN_2_HPP