#include "landscape/mutable_landscape.hpp"

MutableLandscape::MutableLandscape()
    : qualityMap(network), coordsMap(network), probabilityMap(network) {}

MutableLandscape::MutableLandscape(const MutableLandscape &)
    : qualityMap(network), coordsMap(network), probabilityMap(network) {
    assert(false &&
           "No fucking copy constructor and no RVO, fat chance!, use G++ or "
           "wait the upgrade of LEMON for STL compliant code");
};
MutableLandscape::MutableLandscape(MutableLandscape &&)
    : qualityMap(network), coordsMap(network), probabilityMap(network) {
    assert(false &&
           "No fucking move constructor and no RVO, fat chance!, use G++ or "
           "wait the upgrade of LEMON for STL compliant code");
};

std::pair<
    std::unique_ptr<MutableLandscape::Graph::NodeMap<MutableLandscape::Node>>,
    std::unique_ptr<MutableLandscape::Graph::ArcMap<MutableLandscape::Arc>>>
MutableLandscape::copy(const MutableLandscape & landscape) {
    const MutableLandscape::Graph & orig_network = landscape.getNetwork();
    const MutableLandscape::QualityMap & orig_qualityMap =
        landscape.getQualityMap();
    const MutableLandscape::CoordsMap & orig_coordsMap =
        landscape.getCoordsMap();
    const MutableLandscape::ProbabilityMap & orig_difficultyMap =
        landscape.getProbabilityMap();

    lemon::DigraphCopy<MutableLandscape::Graph, MutableLandscape::Graph> cg(
        orig_network, network);

    cg.nodeMap(orig_qualityMap, qualityMap);
    cg.nodeMap(orig_coordsMap, coordsMap);
    cg.arcMap(orig_difficultyMap, probabilityMap);

    auto refs_pair = std::make_pair(
        std::make_unique<
            MutableLandscape::Graph::NodeMap<MutableLandscape::Node>>(
            orig_network),
        std::make_unique<
            MutableLandscape::Graph::ArcMap<MutableLandscape::Arc>>(
            orig_network));

    cg.nodeRef(*refs_pair.first);
    cg.arcRef(*refs_pair.second);

    cg.run();

    return refs_pair;
}

MutableLandscape::~MutableLandscape() {}

MutableLandscape::Node MutableLandscape::addNode(double quality, Point coords) {
    MutableLandscape::Node u = network.addNode();
    this->setQuality(u, quality);
    this->setCoords(u, coords);
    return u;
}
MutableLandscape::Arc MutableLandscape::addArc(MutableLandscape::Node s,
                                               MutableLandscape::Node t,
                                               double probability) {
    MutableLandscape::Arc a = network.addArc(s, t);
    this->setProbability(a, probability);
    return a;
}

void MutableLandscape::removeNode(MutableLandscape::Node u) {
    network.erase(u);
}
void MutableLandscape::removeArc(MutableLandscape::Arc a) { network.erase(a); }

void MutableLandscape::changeSource(MutableLandscape::Arc a,
                                    MutableLandscape::Node s) {
    network.changeSource(a, s);
}
void MutableLandscape::changeTarget(MutableLandscape::Arc a,
                                    MutableLandscape::Node t) {
    network.changeTarget(a, t);
}

void MutableLandscape::setQuality(MutableLandscape::Node u, double quality) {
    qualityMap[u] = quality;
}
void MutableLandscape::setCoords(MutableLandscape::Node u, Point coords) {
    coordsMap[u] = coords;
}
void MutableLandscape::setProbability(MutableLandscape::Arc a,
                                      double probability) {
    probabilityMap[a] = probability;
}

const MutableLandscape::Graph & MutableLandscape::getNetwork() const {
    return network;
}
const MutableLandscape::QualityMap & MutableLandscape::getQualityMap() const {
    return qualityMap;
}
const MutableLandscape::CoordsMap & MutableLandscape::getCoordsMap() const {
    return coordsMap;
}
const MutableLandscape::ProbabilityMap & MutableLandscape::getProbabilityMap()
    const {
    return probabilityMap;
}

const double & MutableLandscape::getQuality(MutableLandscape::Node u) const {
    return qualityMap[u];
}
const Point & MutableLandscape::getCoords(MutableLandscape::Node u) const {
    return coordsMap[u];
}
const double & MutableLandscape::getProbability(MutableLandscape::Arc a) const {
    return probabilityMap[a];
}

double & MutableLandscape::getQualityRef(MutableLandscape::Node u) {
    return qualityMap[u];
}
Point & MutableLandscape::getCoordsRef(MutableLandscape::Node u) {
    return coordsMap[u];
}
double & MutableLandscape::getProbabilityRef(MutableLandscape::Arc a) {
    return probabilityMap[a];
}
