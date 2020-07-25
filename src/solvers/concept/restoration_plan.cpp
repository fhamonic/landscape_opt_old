#include "solvers/concept/restoration_plan.hpp"

RestorationPlan::RestorationPlan(const Landscape & l) : 
        landscape(l),
        _nodeMap(l.getNetwork()), 
        _arcMap(l.getNetwork()) {}
RestorationPlan::~RestorationPlan() {
    for(Option * option : _options)
        delete option;
}

const Landscape & RestorationPlan::getLandscape() const { return landscape; }

RestorationPlan::Option * RestorationPlan::addOption() {
    Option * option = new Option(*this);
    _options.push_back(option);
    return option;
}

int RestorationPlan::indexOfOption(RestorationPlan::Option * option) {
    for(int i=0; (size_t)i<_options.size(); i++)
        if(_options[i] == option)
            return i;
    return -1;
}

void RestorationPlan::removeOption(int index) {
    const int last_index = _options.size() - 1;

    Option * option = _options[index];

    if(last_index > 0 && index != last_index) {
        std::swap(_options[index], _options[last_index]);
    }
    _options.pop_back();

    for(Graph_t::Node v : option->nodes())
        _nodeMap[v].remove(option);
    for(Graph_t::Arc a : option->arcs())
        _arcMap[a].remove(option);

    delete option;
}

void RestorationPlan::removeOption(RestorationPlan::Option * option) {
    int index = indexOfOption(option);
    assert(index > 0);
    removeOption(index);            
}

void RestorationPlan::removeEmptyOptions() {
    for(int i=0; (size_t)i<_options.size(); i++) {
        Option * option = _options[i];
        if(option->getNbElems() > 0)
            continue;
        removeOption(i);
        i--;
    }
}

const std::vector<RestorationPlan::Option*> & RestorationPlan::options() const { return _options; }
int RestorationPlan::getNbOptions() const { return _options.size(); }

void RestorationPlan::notifyAddNode(RestorationPlan::Option* option, Graph_t::Node v) {
    _nodeMap[v].push_back(option);
}
void RestorationPlan::notifyAddArc(RestorationPlan::Option* option, Graph_t::Arc a) {
    _arcMap[a].push_back(option);
}
void RestorationPlan::notifyRemoveNode(RestorationPlan::Option* option, Graph_t::Node v) {
    if(!landscape.getNetwork().valid(v))
        return;
    _nodeMap[v].remove(option);
}
void RestorationPlan::notifyRemoveArc(RestorationPlan::Option* option, Graph_t::Arc a) {
    if(!landscape.getNetwork().valid(a))
        return;
    _arcMap[a].remove(option);
}

void RestorationPlan::remove(Graph_t::Node v) {
    for(RestorationPlan::Option * option : _nodeMap[v])
        option->removePatch(v);
    _nodeMap[v].clear();
}
void RestorationPlan::remove(Graph_t::Arc a) {
    for(RestorationPlan::Option * option : _arcMap[a])
        option->removeLink(a);
    _arcMap[a].clear();
}

const std::list<RestorationPlan::Option*> & RestorationPlan::getOptions(Graph_t::Node v) const { return _nodeMap[v]; }
const std::list<RestorationPlan::Option*> & RestorationPlan::getOptions(Graph_t::Arc a) const { return _arcMap[a]; }

bool RestorationPlan::contains(Graph_t::Node v) const { return _nodeMap[v].size() > 0; }
bool RestorationPlan::contains(Graph_t::Arc a) const { return _arcMap[a].size() > 0; }

void RestorationPlan::cleanInvalidElements() {
    std::vector<Graph_t::Node> deleted_nodes;
    std::vector<Graph_t::Arc> deleted_arcs;
    for(RestorationPlan::Option * option : _options) {
        for(Graph_t::Node v : option->nodes())
            if(!landscape.getNetwork().valid(v)) 
                deleted_nodes.push_back(v);
        for(Graph_t::Arc a : option->arcs())
            if(!landscape.getNetwork().valid(a))
                deleted_arcs.push_back(a);
            
        for(Graph_t::Node v : deleted_nodes)
            option->removePatch(v);
        for(Graph_t::Arc a : deleted_arcs)
            option->removeLink(a);

        deleted_nodes.clear();
        deleted_arcs.clear();
    }
}

void RestorationPlan::print() const {
    const Graph_t & graph = landscape.getNetwork();

    for(Option * option : _options) {
        std::cout << "id: " << option->getId() << " " << option->getCost() << " " << option->getNbElems() << std::endl;

        for(Graph_t::Node u : option->nodes()) {
            std::cout << "\tn " << graph.id(u) << " " << option->getQualityGain(u) << std::endl;
            for(Option * sub_opption : getOptions(u))
                std::cout << "\t\tref: " << sub_opption->getId() << std::endl;
        }
        for(Graph_t::Arc a : option->arcs()) {
            Graph_t::Node source = graph.source(a);
            Graph_t::Node target = graph.target(a);
            std::cout << "\ta " << graph.id(source) << " " << graph.id(target) << " " << option->getRestoredProbability(a) << std::endl;
            for(Option * sub_opption : getOptions(a))
                std::cout << "\t\tref: " << sub_opption->getId() << std::endl;
        }
    }
}


std::ostream & operator<<(std::ostream & in, const RestorationPlan & plan) {
    const Landscape & landscape = plan.getLandscape();
    const Graph_t & graph = landscape.getNetwork();

    for(RestorationPlan::Option * option : plan.options()) {
        in << option->getCost() << " " << option->getNbElems() << std::endl;

        for(Graph_t::Node u : option->nodes()) {
            in << "\tn " << graph.id(u) << " " << option->getQualityGain(u) << std::endl;
        }
        for(Graph_t::Arc a : option->arcs()) {
            Graph_t::Node source = graph.source(a);
            Graph_t::Node target = graph.target(a);
            in << "\ta " << graph.id(source) << " " << graph.id(target) << " " << option->getRestoredProbability(a) << std::endl;
        }
    }

    return in;
}