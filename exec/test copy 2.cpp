#include <iostream>
#include <filesystem>
#include <fstream>

#include "lemon/dijkstra.h"
#include "lemon/connectivity.h"

#include "lemon/graph_to_eps.h"

#include "solvers/pl_eca_2.hpp"
#include "solvers/pl_eca_3.hpp"

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

#include "utils/random_chooser.hpp"
#include "algorithms/multiplicative_dijkstra.h"

#include "helper.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

void parse_quebec(Landscape & landscape, double pow, double median) {
    auto p = [median, pow] (const double d) { return std::exp(std::pow(d,pow)/std::pow(median, pow)*std::log(0.5)); };
    
    std::vector<Graph_t::Node> node_correspondance;
    
    io::CSVReader<3> patches("data/quebec_leam_v3/raw/sommets_leam_v3.csv");
    patches.read_header(io::ignore_extra_column, "area","xcoord","ycoord");
    double area, xcoord, ycoord;
    while(patches.read_row(area, xcoord, ycoord)) {
        Graph_t::Node u = landscape.addNode(area, Point(xcoord,ycoord));
        node_correspondance.push_back(u);
    }

    io::CSVReader<3> links("data/quebec_leam_v3/raw/aretes_leam_v3.csv");
    links.read_header(io::ignore_extra_column, "from","to","Dist");
    int from, to;
    double Dist;
    while(links.read_row(from, to, Dist)) {
        Graph_t::Node u = node_correspondance[from-1];
        Graph_t::Node v = node_correspondance[to-1];
        if(u == lemon::INVALID || v == lemon::INVALID) continue;
        double probability = p(Dist);
        landscape.addArc(u, v, probability);
        landscape.addArc(v, u, probability);
    }
}



Landscape * make_landscape(const Landscape & landscape, Graph_t::Node center, int nb_nodes) {
    const Graph_t & graph = landscape.getNetwork();

    Graph_t::NodeMap<Graph_t::Node> nodes(graph, lemon::INVALID);

    Landscape * new_landscape = new Landscape();
    const Graph_t & new_graph = new_landscape->getNetwork();
        
    const Graph_t::ArcMap<double> & probabilityMap = landscape.getProbabilityMap();
    lemon::MultiplicativeDijkstra<Graph_t, Graph_t::ArcMap<double>> dijkstra(graph, probabilityMap);
    dijkstra.init();
    dijkstra.addSource(center);

    int cpt = 0;
    while (!dijkstra.emptyQueue()) {
        Graph_t::Node x = dijkstra.processNextNode();
        if(cpt >= nb_nodes)
            continue;
        nodes[x] = new_landscape->addNode(landscape.getQuality(x), landscape.getCoords(x));
        cpt++;
    }
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
        Graph_t::Node u = nodes[graph.source(a)];
        Graph_t::Node v = nodes[graph.target(a)];
        if(! (new_graph.valid(u) && new_graph.valid(v)))
            continue;
        new_landscape->addArc(u, v, landscape.getProbability(a));
    }

    return new_landscape;
}

int main() {
    Landscape landscape;
    parse_quebec(landscape, 1, 2300);
    const Graph_t & graph = landscape.getNetwork();

    MyContractionAlgorithm alg2;

    RandomChooser<Graph_t::Node> node_chooser;
    for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u)
        node_chooser.add(u, 1);

    const int n_to_test[] = {50, 250, 500, 1000};
    const int nb_de_chaque = 5;
    const int percent_arcs_to_test[] = {50};

    for(int n : n_to_test) {
        for(int percent_arcs : percent_arcs_to_test) {
            for(int i=0; i<nb_de_chaque; i++) {
                Graph_t::Node center = node_chooser.pick();
                Landscape * sub_landscape = make_landscape(landscape, center, n);
                const Graph_t & sub_graph = sub_landscape->getNetwork();

                Graph_t::ArcMap<bool> deja_select(sub_graph, false);
                RestorationPlan<Landscape> plan(*sub_landscape);

                for(Graph_t::ArcIt a(sub_graph); a != lemon::INVALID; ++a) {
                    if(deja_select[a]) continue;

                    Graph_t::Arc reverse_a = lemon::findArc(sub_graph, sub_graph.target(a), sub_graph.source(a));

                    deja_select[a] = true;
                    deja_select[reverse_a] = true;

                    RestorationPlan<Landscape>::Option option = plan.addOption(15 * -std::log(sub_landscape->getProbability(a)));
                    plan.addArc(option, a, (1 + sub_landscape->getProbability(a)) / 2 );
                    plan.addArc(option, reverse_a, (1 + sub_landscape->getProbability(a)) / 2);
                }

                StdLandscapeParser::get().write(*sub_landscape, "instances", "quebec-" + std::to_string(n) + "-" + std::to_string(i));
                StdRestorationPlanParser plan_parser(*sub_landscape);
                plan_parser.write(plan, "instances", "quebec-" + std::to_string(n) + "-" + std::to_string(i));

                Helper::printInstance(*sub_landscape, plan, "instances/quebec-" + std::to_string(n) + "-" + std::to_string(i) + ".eps");

                delete sub_landscape;
            }
        }
    }

    return EXIT_SUCCESS;
}