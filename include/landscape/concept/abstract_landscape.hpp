/**
 * @file abstract_landscape.hpp
 * @author François Hamonic (francois.hamonic@gmail.com)
 * @brief
 * @version 0.1
 * @date 2020-05-08
 */

#ifndef ABSTRACT_LANDSCAPE_HPP
#define ABSTRACT_LANDSCAPE_HPP

#include "lemon/adaptors.h"
#include "lemon/maps.h"

#include "lemon/bfs.h"

#include "lemon/dim2.h"
using Point = lemon::dim2::Point<double>;

namespace concepts {
template <class GR, typename QM = typename GR::template NodeMap<double>,
          typename PM = typename GR::template ArcMap<double>,
          typename CM = typename GR::template NodeMap<Point>>
class Landscape {
public:
    using Graph = GR;
    using QualityMap = QM;
    using ProbabilityMap = PM;
    using CoordsMap = CM;

    using Node = typename GR::Node;
    using Arc = typename GR::Arc;
    using NodeIt = typename GR::NodeIt;
    using ArcIt = typename GR::ArcIt;
};

/*template <class LS, class GR=typename LS::Graph, typename QM=typename
LS::QualityMap, typename PM=typename LS::ProbabilityMap, typename CM=typename
LS::CoordsMap> concept IsLandscape = std::is_base_of<AbstractLandscape<GR, QM,
PM, CM>, LS>::value;*/ //c++20
}  // namespace concepts

// template <class Graph>
// std::ostream & operator<<(std::ostream & in, const
// concepts::AbstractLandscape<Graph> & output) {
//     const Graph & g = output.getNetwork();

//     in << lemon::countNodes(g) << std::endl;

//     typename Graph::template NodeMap<int> num(g);

//     for (typename Graph::NodeIt u(g); u != lemon::INVALID; ++u) {
//         Point coords = output.getCoords(u);
//         double quality = output.getQuality(u);
//         in << coords[0] << "\t" << coords[1] << "\t" << quality << std::endl;
//     }

//     for (typename Graph::ArcIt a(g); a != lemon::INVALID; ++a) {
//         double probability = output.getProbability(a);
//         int num_from = g.id(g.source(a));
//         int num_to = g.id(g.target(a));
//         in << num_from << "\t" << num_to << "\t" << probability << std::endl;
//     }

//     return in;
// }

#endif  // ABSTRACT_LANDSCAPE_HPP