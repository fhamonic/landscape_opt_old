#include "landscape/landscape.hpp"

Landscape::Landscape() : 
        qualityMap(network),
        coordsMap(network),
        probabilityMap(network) {}

Landscape::Landscape(const Landscape&) : 
        qualityMap(network),
        coordsMap(network),
        probabilityMap(network) { assert(false && "No fucking copy constructor and no RVO, fat chance!"); };
Landscape::Landscape(Landscape&&) : 
        qualityMap(network),
        coordsMap(network),
        probabilityMap(network) { assert(false && "No fucking move constructor and no RVO, fat chance!"); };

std::pair<Graph_t::NodeMap<Graph_t::Node>*, Graph_t::ArcMap<Graph_t::Arc>*> Landscape::copy(const Landscape & landscape) {
    const Graph_t & orig_network = landscape.getNetwork();
    const Landscape::QualityMap & orig_qualityMap = landscape.getQualityMap();
    const Landscape::CoordsMap & orig_coordsMap = landscape.getCoordsMap();
    const Landscape::ProbabilityMap & orig_difficultyMap = landscape.getProbabilityMap();

    lemon::DigraphCopy<Graph_t, Graph_t> cg(orig_network, network);

    cg.nodeMap(orig_qualityMap, qualityMap);
    cg.nodeMap(orig_coordsMap, coordsMap);
    cg.arcMap(orig_difficultyMap, probabilityMap);

    Graph_t::NodeMap<Graph_t::Node> * nodesRef = new Graph_t::NodeMap<Graph_t::Node>(orig_network);
    Graph_t::ArcMap<Graph_t::Arc> * arcsRef = new Graph_t::ArcMap<Graph_t::Arc>(orig_network);

    cg.nodeRef(*nodesRef);
    cg.arcRef(*arcsRef);

    cg.run();

    return std::pair<Graph_t::NodeMap<Graph_t::Node>*, Graph_t::ArcMap<Graph_t::Arc>*>(nodesRef, arcsRef);
}

Landscape::~Landscape() {}

Landscape::Node Landscape::addNode(double quality, Point coords) {
    Graph_t::Node u = network.addNode();
    this->setQuality(u, quality);
    this->setCoords(u, coords);
    return u;
}
Landscape::Arc Landscape::addArc(Graph_t::Node s, Graph_t::Node t, double probability) {
    Graph_t::Arc a = network.addArc(s, t);
    this->setProbability(a, probability);
    return a;
}

void Landscape::removeNode(Graph_t::Node u) { network.erase(u); }
void Landscape::removeArc(Graph_t::Arc a) { network.erase(a); }

void Landscape::changeSource(Graph_t::Arc a, Graph_t::Node s) { network.changeSource(a, s); }
void Landscape::changeTarget(Graph_t::Arc a, Graph_t::Node t) { network.changeTarget(a, t); }

void Landscape::setQuality(Graph_t::Node u, double quality) { qualityMap[u] = quality; }
void Landscape::setCoords(Graph_t::Node u, Point coords) { coordsMap[u] = coords; }
void Landscape::setProbability(Graph_t::Arc a, double probability) { probabilityMap[a] = probability; }

const Graph_t & Landscape::getNetwork() const { return network; }
const Landscape::QualityMap & Landscape::getQualityMap() const { return qualityMap; }
const Landscape::CoordsMap & Landscape::getCoordsMap() const { return coordsMap; }
const Landscape::ProbabilityMap & Landscape::getProbabilityMap() const { return probabilityMap; }


const double & Landscape::getQuality(Graph_t::Node u) const { return qualityMap[u]; }
const Point & Landscape::getCoords(Graph_t::Node u) const { return coordsMap[u]; }
const double & Landscape::getProbability(Graph_t::Arc a) const { return probabilityMap[a]; }

double & Landscape::getQualityRef(Graph_t::Node u) { return qualityMap[u]; }
Point & Landscape::getCoordsRef(Graph_t::Node u) { return coordsMap[u]; }
double & Landscape::getProbabilityRef(Graph_t::Arc a) { return probabilityMap[a]; }
