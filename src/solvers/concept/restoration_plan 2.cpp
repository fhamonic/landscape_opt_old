#include "solvers/concept/restoration_plan_2.hpp"

RestorationPlan2::RestorationPlan2(const Landscape & l) : _landscape(l), _nodeMap(l.getNetwork()), _arcMap(l.getNetwork()) {  }
RestorationPlan2::~RestorationPlan2() {  }
        
const Landscape & RestorationPlan2::getLandscape() const { return _landscape; }


int RestorationPlan2::getNbNodes(Option i) const { return _options_nodes[i].size(); }
int RestorationPlan2::getNbArcs(Option i) const { return _options_arcs[i].size(); }
bool RestorationPlan2::contains(Option i, Graph_t::Node v) const { return _options_nodes_idsMap[i].contains(v); }
bool RestorationPlan2::contains(Option i, Graph_t::Arc a) const { return _options_arcs_idsMap[i].contains(a); }

bool RestorationPlan2::contains(Graph_t::Node v) const { return _nodeMap[v].size() > 0; }
bool RestorationPlan2::contains(Graph_t::Arc a) const { return _arcMap[a].size() > 0; }

void RestorationPlan2::addNode(Option i, Graph_t::Node v, double quality_gain) {
    if(contains(i, v)) return;
    _options_nodes[i].push_back(std::pair<Graph_t::Node, double>(v, quality_gain));
    _options_nodes_idsMap[i][v] = _options_nodes[i].size()-1;
    _nodeMap[v][i] = quality_gain;
}
void RestorationPlan2::addLink(Option i, Graph_t::Arc a, double restored_probability) {
    if(contains(i, a)) return;
    _options_arcs[i].push_back(std::pair<Graph_t::Arc, double>(a, restored_probability));
    _options_arcs_idsMap[i][a] = _options_arcs[i].size()-1;
    _arcMap[a][i] = restored_probability;
}

void RestorationPlan2::removePatch(Option i, Graph_t::Node v) {
    if(!contains(i, v)) return;
    const int id = _options_nodes_idsMap[i][v];
    const int last_id = _options_nodes[i].size()-1;
    if(id < last_id)
        std::swap(_options_nodes[i][id], _options_nodes[i][last_id]);
    _options_nodes[i].pop_back();
    _nodeMap[v].erase(i);  
}
void RestorationPlan2::removeLink(Option i, Graph_t::Arc a) {
    if(!contains(i, a)) return;
    const int id = _options_arcs_idsMap[i][a];
    const int last_id = _options_arcs[i].size()-1;
    if(id < last_id)
        std::swap(_options_arcs[i][id], _options_arcs[i][last_id]);
    _options_arcs[i].pop_back(); 
    _arcMap[a].erase(i);
}

void RestorationPlan2::removePatch(Graph_t::Node v) {
    if(!contains(v)) return;
    for(auto const& [i, val] : _nodeMap[v]) {
        const int id = _options_nodes_idsMap[i][v];
        const int last_id = _options_nodes[i].size()-1;
        if(id < last_id)
            std::swap(_options_nodes[i][id], _options_nodes[i][last_id]);
        _options_nodes[i].pop_back();
    }
    _nodeMap[v].clear();
}
void RestorationPlan2::removeLink(Graph_t::Arc a) {
    if(!contains(a)) return;
    for(auto const& [i, val] : _arcMap[a]) {
        const int id = _options_arcs_idsMap[i][a];
        const int last_id = _options_arcs[i].size()-1;
        if(id < last_id)
            std::swap(_options_arcs[i][id], _options_arcs[i][last_id]);
        _options_arcs[i].pop_back();
    }
    _arcMap[a].clear();
}

double RestorationPlan2::getQualityGain(Option i, Graph_t::Node v) const { return _nodeMap[v].at(i); }
double RestorationPlan2::getRestoredProbability(Option i, Graph_t::Arc a) const { return _arcMap[a].at(i); }

double RestorationPlan2::id(Option i, Graph_t::Node v) const { return _options_nodes_idsMap[i].at(v); }
double RestorationPlan2::id(Option i, Graph_t::Arc a) const { return _options_arcs_idsMap[i].at(a); }


RestorationPlan2::Option RestorationPlan2::addOption(double cost) {
    _options_nodes.push_back(std::vector<std::pair<Graph_t::Node, double>>());
    _options_arcs.push_back(std::vector<std::pair<Graph_t::Arc, double>>());
    _costs.push_back(cost);
    return _costs.size()-1;
}
void RestorationPlan2::setCost(Option i, double cost) { _costs[i] = cost; }
double RestorationPlan2::getCost(Option i) const { return _costs[i]; }

bool RestorationPlan2::isEmpty(Option i) const { return _options_nodes[i].size()>0 || _options_arcs[i].size()>0; }
void RestorationPlan2::removeOption(Option i) {  }
int RestorationPlan2::getNbOptions() const { return _costs.size(); }

const std::map<RestorationPlan2::Option, double> & RestorationPlan2::getOptions(Graph_t::Node v) const { return _nodeMap[v]; }
const std::map<RestorationPlan2::Option, double> & RestorationPlan2::getOptions(Graph_t::Arc a) const { return _arcMap[a]; }

void RestorationPlan2::cleanInvalidElements() {  }