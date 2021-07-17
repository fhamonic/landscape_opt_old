#include <iostream>
#include <filesystem>
#include <fstream>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

struct quality_t {
    using kind = boost::vertex_property_tag;
};
struct probability_t {
    using kind = boost::edge_property_tag;
};

using QualityProperty = boost::property<quality_t, double>;
using ProbabilityProperty = boost::property<probability_t, double>;
using Graph = boost::adjacency_list<boost::vecS, boost::vecS,
        boost::bidirectionalS, QualityProperty, ProbabilityProperty>;
using QualityMap = boost::property_map<Graph, quality_t>::type;
using ProbabilityMap = boost::property_map<Graph, probability_t>::type;
using Vertex = Graph::vertex_descriptor;
using Edge = Graph::edge_descriptor;

void contract_edge(Graph & g, Edge uv) {
    QualityMap qualityMap = boost::get(quality_t(), g);
    const ProbabilityMap probabilityMap = boost::get(probability_t(), g);
    const Vertex u = boost::source(uv, g);
    const Vertex v = boost::target(uv, g);
    const double p = probabilityMap[uv];

    boost::graph_traits<Graph>::in_edge_iterator wu, wu_end;
    for(boost::tie(wu,wu_end) = boost::in_edges(u, g); wu != wu_end; ++wu) {
        Vertex w = boost::source(*wu, g);
        boost::add_edge(w, v, p*probabilityMap[*wu], g);
    }
    qualityMap[v] = p*qualityMap[u];
    boost::remove_vertex(u, g);
}

int main() {
    Graph g;
    QualityMap qualityMap = boost::get(quality_t(), g);
    ProbabilityMap probabilityMap = boost::get(probability_t(), g);

    Vertex u = boost::add_vertex(33, g);
    Vertex v = boost::add_vertex(12, g);
    Vertex w = boost::add_vertex(7, g);

    Edge uv = boost::add_edge(u, v, 0, g).first;
    
    Graph::vertex_iterator v_it, v_end;
    boost::tie(v_it, v_end) = boost::vertices(g);
    std::for_each(v_it, v_end, [&g, &qualityMap, &probabilityMap] (const Vertex s) {

        std::cout << s << " " << qualityMap[s] << std::endl;

        std::vector<Vertex> p(boost::num_vertices(g));
        std::vector<int> d(boost::num_vertices(g));

        boost::dijkstra_shortest_paths(g, s, &p[0], &d[0], probabilityMap, get(boost::vertex_index, g), 
                std::greater<double>(), std::multiplies<double>(), 0, 1,
                boost::default_dijkstra_visitor());

    });

    return EXIT_SUCCESS;
}