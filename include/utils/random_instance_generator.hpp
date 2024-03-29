#ifndef RANDOM_INSRANCE_GENERATOR_HPP
#define RANDOM_INSRANCE_GENERATOR_HPP

#include "landscape/mutable_landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"

#include "utils/random_chooser.hpp"

#include <math.h>
#include <random>
#include <vector>

#include <cassert>

class RandomInstanceGenerator {
public:
    RandomInstanceGenerator(){};
    ~RandomInstanceGenerator(){};

    MutableLandscape * generate_landscape(int seed, int nb_nodes, int nb_arcs,
                                          bool symmetric = true);
    RestorationPlan<MutableLandscape> * generate_plan(
        int seed, const MutableLandscape & landscape, int nb_options,
        bool restore_nodes = false);
};

MutableLandscape * RandomInstanceGenerator::generate_landscape(int seed,
                                                               int nb_nodes,
                                                               int nb_links,
                                                               bool symmetric) {
    using Graph = MutableLandscape::Graph;
    using Node = MutableLandscape::Node;
    using NodeIt = MutableLandscape::NodeIt;
    using NodePair = std::pair<Node, Node>;

    std::default_random_engine gen;
    gen.seed(seed);
    std::uniform_real_distribution<> q_dis(1, 10);
    std::uniform_real_distribution<> p_dis(0, 1);

    MutableLandscape * landscape = new MutableLandscape();
    const Graph & graph = landscape->getNetwork();

    const double angle = 2 * M_PI / nb_nodes;
    for(int i = 0; i < nb_nodes; i++)
        landscape->addNode(q_dis(gen),
                           Point(std::sin(i * angle), std::cos(i * angle)));

    RandomChooser<NodePair> arcs_chooser(seed + 1);

    for(NodeIt u(graph); u != lemon::INVALID; ++u) {
        for(NodeIt v(graph); v != lemon::INVALID; ++v) {
            if(u == v) continue;
            if(symmetric && v < u) continue;
            arcs_chooser.add(NodePair(u, v), 1.0);
        }
    }

    for(int i = 0; i < nb_links; i++) {
        assert(arcs_chooser.canPick());
        NodePair pair = arcs_chooser.pick();
        landscape->addArc(pair.first, pair.second, p_dis(gen));
        if(symmetric) landscape->addArc(pair.second, pair.first, p_dis(gen));
    }

    return landscape;
}

RestorationPlan<MutableLandscape> * RandomInstanceGenerator::generate_plan(
    int seed, const MutableLandscape & landscape, int nb_options,
    bool restore_nodes) {
    using Graph = MutableLandscape::Graph;
    using Node = MutableLandscape::Node;
    using NodeIt = MutableLandscape::NodeIt;
    using Arc = MutableLandscape::Arc;
    using ArcIt = MutableLandscape::ArcIt;

    std::default_random_engine gen;
    gen.seed(seed);
    const int nb_arcs = restore_nodes ? nb_options / 2 : nb_options;

    const Graph & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape> * plan = new RestorationPlan(landscape);

    RandomChooser<Arc> arcs_chooser(seed + 1);
    for(ArcIt a(graph); a != lemon::INVALID; ++a) arcs_chooser.add(a, 1.0);

    for(int i = 0; i < nb_arcs; i++) {
        assert(arcs_chooser.canPick());
        Arc a = arcs_chooser.pick();
        std::uniform_real_distribution<> p_dis(landscape.getProbability(a), 1);

        RestorationPlan<MutableLandscape>::Option option = plan->addOption(1);
        plan->addArc(option, a, p_dis(gen));
    }

    if(!restore_nodes) return plan;

    double avg_quality = 0;
    RandomChooser<Node> nodes_chooser(seed + 2);
    for(NodeIt u(graph); u != lemon::INVALID; ++u) {
        nodes_chooser.add(u, 1.0);
        avg_quality += landscape.getQuality(u);
    }
    avg_quality /= lemon::countNodes(graph);
    for(int i = nb_arcs; i < nb_options; i++) {
        Node u = nodes_chooser.pick();
        nodes_chooser.reset();
        std::uniform_real_distribution<> q_dis(0, avg_quality);

        RestorationPlan<MutableLandscape>::Option option = plan->addOption(1);
        plan->addNode(option, u, q_dis(gen));
    }

    return plan;
}

#endif  // RANDOM_INSRANCE_GENERATOR_HPP