#ifndef RESTORATION_PLAN_2_HPP
#define RESTORATION_PLAN_2_HPP

#include<map>
#include<list>
#include<memory>

#include"landscape/landscape.hpp"

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
        RestorationPlan2(const Landscape & l);
        ~RestorationPlan2();
        
        const Landscape & getLandscape() const;


        int getNbNodes(Option i) const;
        int getNbArcs(Option i) const;
        bool contains(Option i, Graph_t::Node v) const;
        bool contains(Option i, Graph_t::Arc a) const;

        bool contains(Graph_t::Node v) const;
        bool contains(Graph_t::Arc a) const;

        void addNode(Option i, Graph_t::Node v, double quality_gain);
        void addLink(Option i, Graph_t::Arc a, double restored_probability);

        void removePatch(Option i, Graph_t::Node v);
        void removeLink(Option i, Graph_t::Arc a);

        void removePatch(Graph_t::Node v);
        void removeLink(Graph_t::Arc a);

        double getQualityGain(Option i, Graph_t::Node v) const;
        double getRestoredProbability(Option i, Graph_t::Arc a) const;

        double id(Option i, Graph_t::Node v) const;
        double id(Option i, Graph_t::Arc a) const;


        Option addOption(double cost);
        void setCost(Option i, double cost);
        double getCost(Option i) const;

        bool isEmpty(Option i) const;
        void removeOption(Option i);
        int getNbOptions() const;
        
        const std::map<Option, double> & getOptions(Graph_t::Node v) const;
        const std::map<Option, double> & getOptions(Graph_t::Arc a) const;

        void cleanInvalidElements();
};

std::ostream & operator<<(std::ostream & in, const RestorationPlan2 & plan);

#endif //RESTORATION_PLAN_2_HPP