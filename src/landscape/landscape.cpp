#include "landscape/mutable_landscape.hpp"

MutableLandscape::MutableLandscape() : 
        qualityMap(network),
        coordsMap(network),
        probabilityMap(network) {}

MutableLandscape::MutableLandscape(const MutableLandscape&) : 
        qualityMap(network),
        coordsMap(network),
        probabilityMap(network) { assert(false && "No fucking copy constructor and no RVO, fat chance!, use G++ or wait the upgrade of LEMON for STL compliant code"); };
MutableLandscape::MutableLandscape(MutableLandscape&&) : 
        qualityMap(network),
        coordsMap(network),
        probabilityMap(network) { assert(false && "No fucking move constructor and no RVO, fat chance!, use G++ or wait the upgrade of LEMON for STL compliant code"); };

std::pair<std::unique_ptr<Graph_t::NodeMap<Graph_t::Node>>, std::unique_ptr<Graph_t::ArcMap<Graph_t::Arc>>> MutableLandscape::copy(const MutableLandscape & landscape) {
    const Graph_t & orig_network = landscape.getNetwork();
    const MutableLandscape::QualityMap & orig_qualityMap = landscape.getQualityMap();
    const MutableLandscape::CoordsMap & orig_coordsMap = landscape.getCoordsMap();
    const MutableLandscape::ProbabilityMap & orig_difficultyMap = landscape.getProbabilityMap();

    lemon::DigraphCopy<Graph_t, Graph_t> cg(orig_network, network);

    cg.nodeMap(orig_qualityMap, qualityMap);
    cg.nodeMap(orig_coordsMap, coordsMap);
    cg.arcMap(orig_difficultyMap, probabilityMap);

    auto refs_pair = std::make_pair(
            std::make_unique<Graph_t::NodeMap<Graph_t::Node>>(orig_network),
            std::make_unique<Graph_t::ArcMap<Graph_t::Arc>>(orig_network));

    cg.nodeRef(*refs_pair.first);
    cg.arcRef(*refs_pair.second);

    cg.run();

    return refs_pair;
}

MutableLandscape::~MutableLandscape() {}

MutableLandscape::Node MutableLandscape::addNode(double quality, Point coords) {
    Graph_t::Node u = network.addNode();
    this->setQuality(u, quality);
    this->setCoords(u, coords);
    return u;
}
MutableLandscape::Arc MutableLandscape::addArc(Graph_t::Node s, Graph_t::Node t, double probability) {
    Graph_t::Arc a = network.addArc(s, t);
    this->setProbability(a, probability);
    return a;
}

void MutableLandscape::removeNode(Graph_t::Node u) { network.erase(u); }
void MutableLandscape::removeArc(Graph_t::Arc a) { network.erase(a); }

void MutableLandscape::changeSource(Graph_t::Arc a, Graph_t::Node s) { network.changeSource(a, s); }
void MutableLandscape::changeTarget(Graph_t::Arc a, Graph_t::Node t) { network.changeTarget(a, t); }

void MutableLandscape::setQuality(Graph_t::Node u, double quality) { qualityMap[u] = quality; }
void MutableLandscape::setCoords(Graph_t::Node u, Point coords) { coordsMap[u] = coords; }
void MutableLandscape::setProbability(Graph_t::Arc a, double probability) { probabilityMap[a] = probability; }

const Graph_t & MutableLandscape::getNetwork() const { return network; }
const MutableLandscape::QualityMap & MutableLandscape::getQualityMap() const { return qualityMap; }
const MutableLandscape::CoordsMap & MutableLandscape::getCoordsMap() const { return coordsMap; }
const MutableLandscape::ProbabilityMap & MutableLandscape::getProbabilityMap() const { return probabilityMap; }


const double & MutableLandscape::getQuality(Graph_t::Node u) const { return qualityMap[u]; }
const Point & MutableLandscape::getCoords(Graph_t::Node u) const { return coordsMap[u]; }
const double & MutableLandscape::getProbability(Graph_t::Arc a) const { return probabilityMap[a]; }

double & MutableLandscape::getQualityRef(Graph_t::Node u) { return qualityMap[u]; }
Point & MutableLandscape::getCoordsRef(Graph_t::Node u) { return coordsMap[u]; }
double & MutableLandscape::getProbabilityRef(Graph_t::Arc a) { return probabilityMap[a]; }
