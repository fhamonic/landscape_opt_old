#include "landscape/decored_landscape.hpp"

DecoredLandscape::DecoredLandscape(const Graph_t & original_network, const Graph_t::NodeMap<double> & original_qualityMap, const Graph_t::ArcMap<double> & original_difficultyMap, const CoordsMap & coordsMap) : 
        network(original_network),
        original_qualityMap(original_qualityMap), 
        original_probabilityMap(original_difficultyMap), 
        coordsMap(coordsMap),
        qualityMap(network),
        probabilityMap(network) {

    reset();
}
DecoredLandscape::DecoredLandscape(const Landscape & landscape) : 
        network(landscape.getNetwork()),
        original_qualityMap(landscape.getQualityMap()), 
        original_probabilityMap(landscape.getProbabilityMap()), 
        coordsMap(landscape.getCoordsMap()),
        qualityMap(network),
        probabilityMap(network) {

    reset();
}

DecoredLandscape::~DecoredLandscape() {}

const double & DecoredLandscape::getOriginalQuality(Graph_t::Node u) const { return original_qualityMap[u]; }
const double & DecoredLandscape::getOriginalProbability(Graph_t::Arc a) const { return original_probabilityMap[a]; }

const Graph_t & DecoredLandscape::getNetwork() const { return network; }
const DecoredLandscape::QualityMap & DecoredLandscape::getQualityMap() const { return qualityMap; }
const DecoredLandscape::CoordsMap & DecoredLandscape::getCoordsMap() const { return coordsMap; }
const DecoredLandscape::ProbabilityMap & DecoredLandscape::getProbabilityMap() const { return probabilityMap; }

const double & DecoredLandscape::getQuality(Graph_t::Node u) const { return qualityMap[u]; }
const Point & DecoredLandscape::getCoords(Graph_t::Node u) const { return coordsMap[u]; }
const double & DecoredLandscape::getProbability(Graph_t::Arc a) const { return probabilityMap[a]; }

double & DecoredLandscape::getQualityRef(Graph_t::Node u) { return qualityMap[u]; }
double & DecoredLandscape::getProbabilityRef(Graph_t::Arc a) { return probabilityMap[a]; }

void DecoredLandscape::setQuality(Graph_t::Node u, double v) { qualityMap[u] = v; }
void DecoredLandscape::setProbability(Graph_t::Arc a, double v) { probabilityMap[a] = v; }

void DecoredLandscape::reset() {
    for(Graph_t::NodeIt u(network); u != lemon::INVALID; ++u)
        setQuality(u, original_qualityMap[u]);
    for(Graph_t::ArcIt a(network); a != lemon::INVALID; ++a)
        setProbability(a, original_probabilityMap[a]);
}

void DecoredLandscape::apply(const RestorationPlan<Landscape>& plan, RestorationPlan<Landscape>::Option i, double coef) {
    if(!plan.contains(i)) return;
    if(coef == 0.0) return;
    for(auto const& [v, quality_gain] : plan.getNodes(i)) {
        qualityMap[v] += coef * quality_gain;;
    }
    for(auto const& [a, restored_probability] : plan.getArcs(i)) {
        const double interpolated_probability = getOriginalProbability(a) + coef * (restored_probability - getOriginalProbability(a));
        probabilityMap[a] = std::max(interpolated_probability, probabilityMap[a]);
    }
}