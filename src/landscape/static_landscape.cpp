#include "landscape/static_landscape.hpp"

StaticLandscape::StaticLandscape()
    : qualityMap(network), coordsMap(network), probabilityMap(network) {}

StaticLandscape::~StaticLandscape() {}

const StaticLandscape::Graph & StaticLandscape::getNetwork() const {
    return network;
}
const StaticLandscape::QualityMap & StaticLandscape::getQualityMap() const {
    return qualityMap;
}
const StaticLandscape::CoordsMap & StaticLandscape::getCoordsMap() const {
    return coordsMap;
}
const StaticLandscape::ProbabilityMap & StaticLandscape::getProbabilityMap()
    const {
    return probabilityMap;
}

const double & StaticLandscape::getQuality(StaticLandscape::Node u) const {
    return qualityMap[u];
}
const Point & StaticLandscape::getCoords(StaticLandscape::Node u) const {
    return coordsMap[u];
}
const double & StaticLandscape::getProbability(StaticLandscape::Arc a) const {
    return probabilityMap[a];
}
