#include "solvers/concept/restoration_plan.hpp"

RestorationPlan::Option::Option(RestorationPlan & plan) : _plan(plan), _cost{0}, _id{-1} {}
RestorationPlan::Option::~Option() {}

void RestorationPlan::Option::setId(int id) { _id = id; }
int RestorationPlan::Option::getId() const { return _id; }

int RestorationPlan::Option::getNbNodes() const { return _nodes.size(); }
int RestorationPlan::Option::getNbArcs() const { return _arcs.size(); }
int RestorationPlan::Option::getNbElems() const { return _nodes.size()+_arcs.size(); }

bool RestorationPlan::Option::contains(Graph_t::Node v) const { return _nodeMap.find(v) != _nodeMap.end(); }
bool RestorationPlan::Option::contains(Graph_t::Arc a) const { return _arcMap.find(a) != _arcMap.end(); }

void RestorationPlan::Option::addPatch(Graph_t::Node v, double quality_gain) {
    if(this->contains(v)) return;
    
    _nodes.push_back(v);
    const int node_id = _nodes.size()-1;
    _nodeMap[v] = std::pair<double, int> (quality_gain, node_id);

    _plan.notifyAddNode(this, v); 
}
void RestorationPlan::Option::addLink(Graph_t::Arc a, double restored_probability) {
    if(this->contains(a)) return;

    _arcs.push_back(a);  
    const int arc_id = _arcs.size()-1;
    _arcMap[a] = std::pair<double, int> (restored_probability, arc_id); 

    _plan.notifyAddArc(this, a);
}
void RestorationPlan::Option::setCost(double cost) { _cost = cost; }


void RestorationPlan::Option::removePatch(Graph_t::Node v) {
    if(!this->contains(v)) return;
    const int node_id = id(v);
    const int last_id = _nodes.size() - 1;
    if(node_id != last_id) {
        std::swap(_nodes[node_id], _nodes[last_id]);
        _nodeMap.at(_nodes[node_id]).second = node_id;
    }
    _nodes.pop_back();
    _nodeMap.erase(v);

    _plan.notifyRemoveNode(this, v);          
}
void RestorationPlan::Option::removeLink(Graph_t::Arc a) {
    if(!this->contains(a)) return;
    const int arc_id = id(a);
    const int last_id = _arcs.size() - 1;
    if(arc_id != last_id) {
        std::swap(_arcs[arc_id], _arcs[last_id]);
        _arcMap.at(_arcs[arc_id]).second = arc_id;
    }
    _arcs.pop_back();
    _arcMap.erase(a);

    _plan.notifyRemoveArc(this, a);
}

double RestorationPlan::Option::getQualityGain(Graph_t::Node v) const { assert(this->contains(v)); return _nodeMap.at(v).first; }
double RestorationPlan::Option::getRestoredProbability(Graph_t::Arc a) const { assert(this->contains(a)); return _arcMap.at(a).first; }
double RestorationPlan::Option::getCost() const { return _cost; }

double & RestorationPlan::Option::getQualityGainRef(Graph_t::Node v) { assert(this->contains(v)); return _nodeMap[v].first; }
double & RestorationPlan::Option::getRestoredProbabilityRef(Graph_t::Arc a) { assert(this->contains(a)); return _arcMap[a].first; }
double & RestorationPlan::Option::getCostRef() { return _cost; }

int RestorationPlan::Option::id(Graph_t::Node v) const { assert(this->contains(v)); return _nodeMap.at(v).second; }
int RestorationPlan::Option::id(Graph_t::Arc a) const { assert(this->contains(a)); return _arcMap.at(a).second; }

const std::vector<Graph_t::Node> & RestorationPlan::Option::nodes() const { return _nodes; }
const std::vector<Graph_t::Arc> & RestorationPlan::Option::arcs() const { return _arcs; }

double RestorationPlan::Option::operator[](Graph_t::Node v) const { return getQualityGain(v); } 
double RestorationPlan::Option::operator[](Graph_t::Arc a) const { return getRestoredProbability(a); }

double & RestorationPlan::Option::operator[](Graph_t::Node v) { return getQualityGainRef(v); } 
double & RestorationPlan::Option::operator[](Graph_t::Arc a) { return getRestoredProbabilityRef(a); }
