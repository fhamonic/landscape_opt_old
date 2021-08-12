#include <filesystem>
#include <fstream>
#include <iostream>

#include "lemon/connectivity.h"
#include "lemon/dijkstra.h"

#include "lemon/graph_to_eps.h"

#include "solvers/pl_eca_2.hpp"
#include "solvers/pl_eca_3.hpp"

#include "parsers/std_mutable_landscape_parser.hpp"
#include "parsers/std_restoration_plan_parser.hpp"

#include "algorithms/multiplicative_dijkstra.hpp"
#include "utils/random_chooser.hpp"

#include "helper.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

void parse_quebec(MutableLandscape & landscape, double pow, double median) {
    auto p = [median, pow](const double d) {
        return std::exp(std::pow(d, pow) / std::pow(median, pow) *
                        std::log(0.5));
    };

    std::vector<MutableLandscape::Node> node_correspondance;

    io::CSVReader<3> patches("data/quebec_leam_v3/raw/sommets_leam_v3.csv");
    patches.read_header(io::ignore_extra_column, "area", "xcoord", "ycoord");
    double area, xcoord, ycoord;
    while(patches.read_row(area, xcoord, ycoord)) {
        MutableLandscape::Node u =
            landscape.addNode(area, Point(xcoord, ycoord));
        node_correspondance.push_back(u);
    }

    io::CSVReader<3> links("data/quebec_leam_v3/raw/aretes_leam_v3.csv");
    links.read_header(io::ignore_extra_column, "from", "to", "Dist");
    int from, to;
    double Dist;
    while(links.read_row(from, to, Dist)) {
        MutableLandscape::Node u = node_correspondance[from - 1];
        MutableLandscape::Node v = node_correspondance[to - 1];
        if(u == lemon::INVALID || v == lemon::INVALID) continue;
        double probability = p(Dist);
        landscape.addArc(u, v, probability);
        landscape.addArc(v, u, probability);
    }
}

MutableLandscape * make_landscape(const MutableLandscape & landscape,
                                  MutableLandscape::Node center, int nb_nodes) {
    const MutableLandscape::Graph & graph = landscape.getNetwork();

    MutableLandscape::Graph::NodeMap<MutableLandscape::Node> nodes(
        graph, lemon::INVALID);

    MutableLandscape * new_landscape = new MutableLandscape();
    const MutableLandscape::Graph & new_graph = new_landscape->getNetwork();

    const MutableLandscape::ProbabilityMap & probabilityMap =
        landscape.getProbabilityMap();
    lemon::MultiplicativeDijkstra<MutableLandscape::Graph,
                                  MutableLandscape::ProbabilityMap>
        dijkstra(graph, probabilityMap);
    dijkstra.init();
    dijkstra.addSource(center);

    int cpt = 0;
    while(!dijkstra.emptyQueue()) {
        MutableLandscape::Node x = dijkstra.processNextNode();
        if(cpt >= nb_nodes) continue;
        nodes[x] = new_landscape->addNode(landscape.getQuality(x),
                                          landscape.getCoords(x));
        cpt++;
    }
    for(MutableLandscape::ArcIt a(graph); a != lemon::INVALID; ++a) {
        MutableLandscape::Node u = nodes[graph.source(a)];
        MutableLandscape::Node v = nodes[graph.target(a)];
        if(!(new_graph.valid(u) && new_graph.valid(v))) continue;
        new_landscape->addArc(u, v, landscape.getProbability(a));
    }

    return new_landscape;
}

int main() {
    std::ofstream data_log("output/data.log");
    data_log << std::fixed << std::setprecision(6);
    data_log << "center "
             << "percent_arcs "
             << "nb_vars "
             << "nb_constraints "
             << "nb_elems "
             << "nb_vars_contract "
             << "nb_constraints_contract "
             << "nb_elems_contract " << std::endl;

    MutableLandscape landscape;
    parse_quebec(landscape, 1, 2300);
    const MutableLandscape::Graph & graph = landscape.getNetwork();

    // StdMutableLandscapeParser::get().write(*landscape, "output",
    // "out_landscape");

    MyContractionAlgorithm alg2;

    RandomChooser<MutableLandscape::Node> node_chooser;
    for(MutableLandscape::NodeIt u(graph); u != lemon::INVALID; ++u)
        node_chooser.add(u, 1);

    const int nb_tests = 20;
    const int n_to_test[] = {500};

    Solvers::PL_ECA_2 * pl_eca_2 = new Solvers::PL_ECA_2();
    (*pl_eca_2).setLogLevel(0);
    Solvers::PL_ECA_3 * pl_eca_3 = new Solvers::PL_ECA_3();
    (*pl_eca_3).setLogLevel(0).setTimeout(3600);

    for(int i = 0; i < nb_tests; i++) {
        MutableLandscape::Node center = node_chooser.pick();
        std::cout << "step " << i << "/" << nb_tests << std::endl;

        for(int n : n_to_test) {
            MutableLandscape * sub_landscape =
                make_landscape(landscape, center, n);
            const MutableLandscape::Graph & sub_graph =
                sub_landscape->getNetwork();

            const int m = lemon::countArcs(sub_graph);
            std::cout << "nodes : 500" << std::endl
                      << "arcs : " << m << std::endl;

            RandomChooser<MutableLandscape::Arc> arc_chooser;
            for(MutableLandscape::ArcIt a(sub_graph); a != lemon::INVALID; ++a)
                arc_chooser.add(a, 1);

            RestorationPlan<MutableLandscape> plan(*sub_landscape);
            for(int percent_arcs = 0; percent_arcs <= 100; percent_arcs += 5) {
                Solution solution2 = pl_eca_2->solve(*sub_landscape, plan, 0);
                Solution solution3 = pl_eca_3->solve(*sub_landscape, plan, 0);

                data_log << graph.id(center) << " " << percent_arcs << " "
                         << solution2.nb_vars << " " << solution2.nb_constraints
                         << " " << solution2.nb_elems << " "
                         << solution3.nb_vars << " " << solution3.nb_constraints
                         << " " << solution3.nb_elems << std::endl;

                if(percent_arcs == 100) break;
                for(int i = 0; i < (m / 20); ++i) {
                    MutableLandscape::Arc a = arc_chooser.pick();
                    RestorationPlan<MutableLandscape>::Option option =
                        plan.addOption(1);
                    plan.addArc(option, a,
                                std::sqrt(sub_landscape->getProbability(a)));
                }
            }
            arc_chooser.reset();

            delete sub_landscape;
        }
    }

    return EXIT_SUCCESS;
}