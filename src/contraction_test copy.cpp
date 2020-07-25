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


double compute_value(const Landscape & landscape, const RestorationPlan & plan, Graph_t::Node t, bool restored) {
    const Graph_t & original_g = landscape.getNetwork();

    Graph_t::ArcMap<double> probabilities(original_g);
    for(Graph_t::ArcIt b(original_g); b != lemon::INVALID; ++b) {
        probabilities[b] = landscape.getProbability(b);
        if(restored)
            for(RestorationPlan::Option * option : plan.getOptions(b))
                probabilities[b] = std::max(probabilities[b], option->getRestoredProbability(b));
    }    

    lemon::MultiplicativeSimplerDijkstra<Graph_t, Graph_t::ArcMap<double>> dijkstra(original_g, probabilities);
    double sum = 0;
    dijkstra.init(t);
    while (!dijkstra.emptyQueue()) {
        std::pair<Graph_t::Node, double> pair = dijkstra.processNextNode();
        Graph_t::Node v = pair.first;
        const double p_tv = pair.second;
        sum += landscape.getQuality(v) * p_tv;
        if(restored)
            for(RestorationPlan::Option * option : plan.getOptions(v))
                sum += option->getQualityGain(v) * p_tv;

    }
    return sum;
}

double compute_value_reversed(const Landscape & landscape, const RestorationPlan & plan, Graph_t::Node t, bool restored) {
    typedef lemon::ReverseDigraph<const Graph_t> Reversed;

    const Graph_t & original_g = landscape.getNetwork();
    Reversed reversed_g(original_g); 

    Graph_t::ArcMap<double> probabilities(original_g);
    for(Graph_t::ArcIt b(original_g); b != lemon::INVALID; ++b) {
        probabilities[b] = landscape.getProbability(b);
        if(restored)
            for(RestorationPlan::Option * option : plan.getOptions(b))
                probabilities[b] = std::max(probabilities[b], option->getRestoredProbability(b));
    }   

    lemon::MultiplicativeSimplerDijkstra<Reversed, Graph_t::ArcMap<double>> dijkstra(reversed_g, probabilities);
    double sum = 0;
    dijkstra.init(t);
    while (!dijkstra.emptyQueue()) {
        std::pair<Graph_t::Node, double> pair = dijkstra.processNextNode();
        Graph_t::Node v = pair.first;
        const double p_tv = pair.second;
        sum += landscape.getQuality(v) * p_tv;
        if(restored)
            for(RestorationPlan::Option * option : plan.getOptions(v))
                sum += option->getQualityGain(v) * p_tv;

    }
    return sum;
}

void display(const Landscape & landscape, const RestorationPlan & plan, Graph_t::Node t, /*std::vector<Graph_t::Arc> & contractable_arcs,*/ std::string name) {
    const Graph_t & graph = landscape.getNetwork();
    
    auto radius = [&] (Graph_t::Node u) { return std::sqrt(landscape.getQuality(u) / (2*M_PI)); };

    Graph_t::NodeMap<int> node_idsMap(graph);
    Graph_t::NodeMap<lemon::Color> node_colorsMap(graph, lemon::WHITE);
    Graph_t::NodeMap<double> node_sizesMap(graph);
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        node_idsMap[v] = graph.id(v);
        node_sizesMap[v] = radius(v);
    }
    node_colorsMap[t] = lemon::BLUE;
    
    Graph_t::ArcMap<lemon::Color> arcs_colorsMap(graph, lemon::BLACK);
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a)
        if(plan.contains(a))
            arcs_colorsMap[a] = lemon::RED;
    // for(Graph_t::Arc a : contractable_arcs)
    //     arcs_colorsMap[a] = plan.contains(a) ? lemon::MAGENTA : lemon::GREEN;

    Graph_t::ArcMap<double> edge_widths(graph, 1);
    
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
            .nodeColors(node_colorsMap)
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
        std::cerr << "input requiered : <landscape_file> <nb_restored_arcs>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];
    const int nb_restored_arcs = std::atoi(argv[2]);

    const double alpha = 6000;
    
    Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);
    const Graph_t & graph = landscape->getNetwork();


    StdRestorationPlanParser parser(*landscape);

    /*
    RestorationPlan * plan = parser.parse("output/probability_test.problem");
    RestorationPlan & splan = *plan;
    /*/
    RestorationPlan splan(*landscape);

    RandomChooser<Graph_t::Arc> arc_chooser;
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a)
        arc_chooser.add(a, 1);
    for(int i=0; i<nb_restored_arcs; i++) {
        Graph_t::Arc a = arc_chooser.pick();
        RestorationPlan::Option * option = splan.addOption();
        option->setId(i);
        option->setCost(1);
        option->addLink(a, ECA::P_func( landscape->getProbability(a) / 1.5 , alpha ));
    }
    parser.write(splan, "output", "probability_test", false);

    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a)
        landscape->setProbability(a, ECA::P_func( landscape->getProbability(a) , alpha ));
    StdLandscapeParser::get().write(*landscape, "output", "probability_test", false);
    //*/




    Helper::assert_well_formed(*landscape, splan);




    std::cout << std::setprecision(10);

    std::chrono::time_point<std::chrono::high_resolution_clock> t0, t1;
    MyContractionAlgorithm alg2;

    t0 = std::chrono::high_resolution_clock::now();
    Graph_t::NodeMap<ContractionResult> * results = alg2.precompute(*landscape, splan);
    t1 = std::chrono::high_resolution_clock::now();



    for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
        ContractionResult result = (*results)[t];
        const double base = compute_value_reversed(*landscape, splan, t, false);
        const double contracted = compute_value_reversed(*result.landscape, *result.plan, result.t, false);
        if(fabs(base - contracted) > 0.0001) {
            std::cout << graph.id(t) << " : " << base << " != " << contracted << std::endl;
        }

        const double base_restored = compute_value_reversed(*landscape, splan, t, true);
        const double contracted_restored = compute_value_reversed(*result.landscape, *result.plan, result.t, true);
        if(fabs(base_restored - contracted_restored) > 0.0001) {
            std::cout << graph.id(t) << " restored : " << base_restored << " != " << contracted_restored << std::endl;
        }
    }

    Graph_t::Node t = graph.nodeFromId(12);
    display(*landscape, splan, t, "original");
    display(*(*results)[t].landscape, *(*results)[t].plan, (*results)[t].t, "contracted");

    std::cout << *(*results)[t].landscape << std::endl;

    
    int sum_of_nb_nodes = 0;
    int sum_of_nb_arcs = 0;
    for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
        ContractionResult result = (*results)[t];
        sum_of_nb_nodes += lemon::countNodes(result.landscape->getNetwork());   
        sum_of_nb_arcs += lemon::countArcs(result.landscape->getNetwork());
    }

    std::cout << "Total nb of nodes : " << sum_of_nb_nodes << std::endl;
    std::cout << "Total nb of arcs : " << sum_of_nb_arcs << std::endl;

    int normal_time_us = std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count();
    std::cout << normal_time_us << std::endl;


    delete results;
    delete landscape;


    return EXIT_SUCCESS;
}
