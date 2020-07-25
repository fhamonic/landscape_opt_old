#ifndef RESTORATION_PLAN_HPP
#define RESTORATION_PLAN_HPP

#include<map>
#include<list>
#include<memory>

#include"landscape/landscape.hpp"

class RestorationPlan {
    public:
        class Option {
            private:
                RestorationPlan & _plan;

                std::map<Graph_t::Node, std::pair<double, int>> _nodeMap;
                std::map<Graph_t::Arc, std::pair<double, int>> _arcMap;
                double _cost;

                std::vector<Graph_t::Node> _nodes;
                std::vector<Graph_t::Arc> _arcs;
                int _id;
            public:
                Option(RestorationPlan & plan);
                ~Option();

                void setId(int id);
                int getId() const;

                int getNbNodes() const;
                int getNbArcs() const;
                int getNbElems() const;

                bool contains(Graph_t::Node v) const;
                bool contains(Graph_t::Arc a) const;

                void addPatch(Graph_t::Node v, double quality);
                void addLink(Graph_t::Arc a, double length);
                void setCost(double cost);

                void removePatch(Graph_t::Node v);
                void removeLink(Graph_t::Arc a);

                double getQualityGain(Graph_t::Node v) const;
                double getRestoredProbability(Graph_t::Arc a) const;
                double getCost() const;

                double & getQualityGainRef(Graph_t::Node v);
                double & getRestoredProbabilityRef(Graph_t::Arc a);
                double & getCostRef();

                int id(Graph_t::Node v) const;
                int id(Graph_t::Arc a) const;
                
                const std::vector<Graph_t::Node> & nodes() const;
                const std::vector<Graph_t::Arc> & arcs() const;

                double operator[](Graph_t::Node v) const;
                double operator[](Graph_t::Arc a) const;
                
                double & operator[](Graph_t::Node v);
                double & operator[](Graph_t::Arc a);
        };
    private:
        const Landscape & landscape;
        std::vector<Option*> _options;

        Graph_t::NodeMap<std::list<Option*>> _nodeMap;
        Graph_t::ArcMap<std::list<Option*>> _arcMap;

        void notifyAddNode(RestorationPlan::Option* option, Graph_t::Node v);
        void notifyAddArc(RestorationPlan::Option* option, Graph_t::Arc a);

        void notifyRemoveNode(RestorationPlan::Option* option, Graph_t::Node v);
        void notifyRemoveArc(RestorationPlan::Option* option, Graph_t::Arc a);
    public:
        RestorationPlan(const Landscape & l);
        ~RestorationPlan();

        const Landscape & getLandscape() const;

        Option * addOption();

        int indexOfOption(Option * option);
        bool containsOption(Option * option) const;

        void removeOption(int index);
        void removeOption(Option * option);

        void removeEmptyOptions();

        const std::vector<Option*> & options() const;
        int getNbOptions() const;
        
        void remove(Graph_t::Node v);
        void remove(Graph_t::Arc a);

        const std::list<Option*> & getOptions(Graph_t::Node v) const;
        const std::list<Option*> & getOptions(Graph_t::Arc a) const;

        bool contains(Graph_t::Node v) const;
        bool contains(Graph_t::Arc a) const;

        void cleanInvalidElements();

        void print() const;
};

std::ostream & operator<<(std::ostream & in, const RestorationPlan & plan);

#endif //RESTORATION_PLAN_HPP