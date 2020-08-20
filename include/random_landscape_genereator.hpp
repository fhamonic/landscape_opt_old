#ifndef RANDOM_LANDSCAPE_GENERATOR_HPP
#define RANDOM_LANDSCAPE_GENERATOR_HPP

#include "landscape/landscape.hpp"
#include "random_chooser.hpp"

#include <random>
#include <vector>

#include <cassert>

class RandomLandscapeGenerator {
    private:
    public:
        RandomLandscapeGenerator() {};
        ~RandomLandscapeGenerator() {};

        Landscape * generate(int seed, int nb_nodes, int nb_arcs, bool symmetric=true);
};

Landscape * RandomLandscapeGenerator::generate(int seed, int nb_nodes, int nb_arcs, bool symmetric) {
    typedef Graph_t Graph;
    typedef Graph_t::Node Node;
    typedef Graph_t::NodeIt NodeIt;
    typedef std::pair<Node, Node> NodePair;

    std::default_random_engine gen;
    gen.seed(seed);
    std::uniform_real_distribution<> q_dis(1, 10);
    std::uniform_real_distribution<> p_dis(0, 1);
    
    Landscape * landscape = new Landscape();
    const Graph & graph = landscape->getNetwork(); 

    const double angle = 2 * math.pi / nb_nodes;
    for(int i=0; i<nb_nodes; i++)
        landscape->addNode(q_dis(gen), Point(std::sin(i * angle),std::cos(i * angle)));

    RandomChooser<NodePair> arcs_chooser(seed+1);

    for(NodeIt u(graph); u!=lemon::INVALID; ++u) {
        for(NodeIt v(graph); v!=lemon::INVALID; ++v) {
            if(u == v) continue;
            arcs_chooser.add(NodePair(u,v), 1.0);
        }
    }

    for(int i=0; i<nb_arcs; i++) {
        assert(arcs_chooser.canPick());
        NodePair pair = arcs_chooser.pick();
        landscape->addArc(pair.a, pair.b, p_dis(gen));
    }
        
    
}



#endif //RANDOM_LANDSCAPE_GENERATOR_HPP