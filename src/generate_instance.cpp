#include <iostream>
#include <filesystem>
#include <fstream>

#include "lemon/dijkstra.h"
#include "lemon/connectivity.h"

#include "lemon/graph_to_eps.h"


#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

#include "random_chooser.hpp"

#include "helper.hpp"

#include "precomputation/my_contraction_algorithm.hpp"


Landscape * make_landscape(const Landscape & landscape, Graph_t::Node center, int nb_nodes) {
    const Graph_t & graph = landscape.getNetwork();

    Graph_t::NodeMap<Graph_t::Node> nodes(graph, lemon::INVALID);

    Landscape * new_landscape = new Landscape();
    const Graph_t & new_graph = new_landscape->getNetwork();
        
    const Graph_t::ArcMap<double> & difficultyMap = landscape.getProbabilityMap();
    lemon::Dijkstra<Graph_t, Graph_t::ArcMap<double>> dijkstra(graph, difficultyMap);
    dijkstra.init();
    dijkstra.addSource(center);

    int cpt = 0;
    while (!dijkstra.emptyQueue()) {
        Graph_t::Node x = dijkstra.processNextNode();
        if(cpt >= nb_nodes)
            break;
        nodes[x] = new_landscape->addNode(landscape.getQuality(x), landscape.getCoords(x));
        cpt++;
    }

    if(cpt < nb_nodes) {
        std::cerr << "not enougth nodes aroud " << graph.id(center) << std::endl;
        delete new_landscape;
        return NULL;
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

void display(const Landscape & landscape, const RestorationPlan & plan, std::string name) {
    const Graph_t & graph = landscape.getNetwork();
    
    auto radius = [&] (Graph_t::Node u) { return std::sqrt(landscape.getQuality(u) / (2*M_PI)); };

    Graph_t::NodeMap<int> node_idsMap(graph);
    Graph_t::NodeMap<double> node_sizesMap(graph);
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        node_idsMap[v] = graph.id(v);
        node_sizesMap[v] = radius(v);
    }
    
    Graph_t::ArcMap<lemon::Color> arcs_colorsMap(graph, lemon::BLACK);
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a)
        if(plan.contains(a))
            arcs_colorsMap[a] = lemon::RED;
    
    const double r_max = Helper::findNodeScale(landscape);
    const double a_max = r_max * r_max;

    std::string title = name + " n=" + std::to_string(lemon::countNodes(graph)) + " , m=" + std::to_string(lemon::countArcs(graph)) + " , #options=" + std::to_string(plan.options().size());

    return lemon::graphToEps(graph, "output/" + name)
            .title(title)
            .coords(landscape.getCoordsMap())
            .absoluteNodeSizes(true)
            .autoNodeScale(false)
            .nodeSizes(node_sizesMap)
            .nodeScale(0.75 * r_max)
            .nodeTextSize(a_max / 2)
            .autoArcWidthScale(true)
            .nodeTexts(node_idsMap)
            .arcColors(arcs_colorsMap)
            .drawArrows(true)
            .arrowLength(r_max / 8)
            .arrowWidth(r_max / 6)
            .enableParallel(true)
            .parArcDist(r_max / 6)
            .border(20)
            .run();
}

int main (int argc, const char *argv[]) {
    if(argc < 2) {
        std::cerr << "input requiered : <landscape_file> <nb_nodes> <nb_restored_arcs> <id_center>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];
    const int nb_nodes = std::atoi(argv[2]);
    const int nb_restored_arcs = std::atoi(argv[3]);
    const int id_center = std::atoi(argv[4]);

    
    Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);
    const Graph_t & graph = landscape->getNetwork();

    Graph_t::Node t = graph.nodeFromId(id_center);

    Landscape * sub_landscape = make_landscape(*landscape, t, nb_nodes);

    if(sub_landscape == nullptr) {
        delete landscape;
        return EXIT_FAILURE;
    }

    const Graph_t & sub_graph = sub_landscape->getNetwork();

    if(nb_restored_arcs > lemon::countArcs(sub_graph)) {
        std::cout << "not enougth arcs centered on " << graph.id(t) << std::endl;
        return EXIT_FAILURE;
    }

    std::string sub_landscape_name = "landscape_" + std::to_string(nb_nodes) + "_" + std::to_string(id_center);

    StdRestorationPlanParser parser(*sub_landscape);
    RestorationPlan * plan = new RestorationPlan(*sub_landscape);

    //Graph_t::ArcMap<int> * centralityMap = new Graph_t::ArcMap<int>(sub_graph, 1); 
    Graph_t::ArcMap<int> * centralityMap = Helper::arcCentralityMap(sub_graph, sub_landscape->getProbabilityMap()); 

    RandomChooser<Graph_t::Arc> arc_chooser;
    for(Graph_t::ArcIt a(sub_graph); a != lemon::INVALID; ++a)
        arc_chooser.add(a, (*centralityMap)[a]);
    for(int i=0; i<nb_restored_arcs; i++) {
        Graph_t::Arc a = arc_chooser.pick();
        RestorationPlan::Option * option = plan->addOption();
        option->setId(i);
        option->setCost(1);
        option->addLink(a, std::sqrt(sub_landscape->getProbability(a)));
    }

    StdLandscapeParser::get().write(*sub_landscape, "output", sub_landscape_name);
    parser.write(*plan, "output", "landscape_" + std::to_string(nb_nodes) + "_" + std::to_string(nb_restored_arcs) + "_" + std::to_string(id_center));

    display(*sub_landscape, *plan, "landscape_" + std::to_string(nb_nodes) + "_" + std::to_string(nb_restored_arcs) + "_" + std::to_string(id_center));

    delete centralityMap;
    delete plan;
    delete sub_landscape;
    delete landscape;

    return EXIT_SUCCESS;
}
