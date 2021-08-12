#include <filesystem>
#include <fstream>
#include <iostream>

#include "lemon/connectivity.h"
#include "lemon/dijkstra.h"

#include "lemon/graph_to_eps.h"

#include "parsers/std_mutable_landscape_parser.hpp"
#include "parsers/std_restoration_plan_parser.hpp"

#include "utils/random_chooser.hpp"

#include "helper.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

template <typename LS>
double compute_value_reversed(const LS & landscape,
                              typename LS::Graph::Node t) {
    using Graph = typename LS::Graph;
    using Node = typename LS::Node;
    using ProbabilityMap = typename LS::ProbabilityMap;
    using QualityMap = typename LS::QualityMap;

    using Reversed = lemon::ReverseDigraph<const Graph>;

    const Graph & original_g = landscape.getNetwork();
    Reversed reversed_g(original_g);

    lemon::MultiplicativeSimplerDijkstra<Reversed, ProbabilityMap> dijkstra(
        reversed_g, landscape.getProbabilityMap());
    double sum = 0;
    dijkstra.init(t);
    while(!dijkstra.emptyQueue()) {
        std::pair<Node, double> pair = dijkstra.processNextNode();
        Node v = pair.first;
        const double p_tv = pair.second;
        sum += landscape.getQuality(v) * p_tv;
    }
    return sum;
}

int main(int argc, const char * argv[]) {
    if(argc < 3) {
        std::cerr << "input requiered : <landscape_file> <plan_file>"
                  << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];
    std::filesystem::path plan_path = argv[2];

    const int seed = 1245;
    std::cout << std::setprecision(10);

    MutableLandscape landscape =
        StdMutableLandscapeParser::get().parse(landscape_path);
    const MutableLandscape::Graph & graph = landscape.getNetwork();
    StdRestorationPlanParser parser(landscape);
    RestorationPlan<MutableLandscape> plan = parser.parse(plan_path);

    Helper::assert_well_formed(landscape, plan);
    const auto nodeOptions = plan.computeNodeOptionsMap();
    const auto arcOptions = plan.computeArcOptionsMap();

    std::chrono::time_point<std::chrono::high_resolution_clock> t0, t1;
    MyContractionAlgorithm algo;

    t0 = std::chrono::high_resolution_clock::now();
    auto results = algo.precompute(landscape, plan);
    t1 = std::chrono::high_resolution_clock::now();

    int normal_time_us =
        std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << "Contraction done in " << normal_time_us << " ms" << std::endl;

    MutableLandscape::Graph::NodeMap<
        RestorationPlan<StaticLandscape>::NodeOptionsMap>
        t_nodeOptions(graph);
    MutableLandscape::Graph::NodeMap<
        RestorationPlan<StaticLandscape>::ArcOptionsMap>
        t_arcOptions(graph);
    for(MutableLandscape::NodeIt t(graph); t != lemon::INVALID; ++t) {
        const ContractionResult & result = *(*results)[t];
        t_nodeOptions[t] = result.plan.computeNodeOptionsMap();
        t_arcOptions[t] = result.plan.computeArcOptionsMap();
    }

    const int n = lemon::countNodes(graph);
    const double epsilon = 0.0001;

    for(MutableLandscape::NodeIt t(graph); t != lemon::INVALID; ++t) {
        const ContractionResult & result = *(*results)[t];
        const double base = compute_value_reversed(landscape, t);
        const double contracted =
            compute_value_reversed(result.landscape, result.t);

        if(fabs(base - contracted) > epsilon) {
            std::cout << graph.id(t) << " : " << base << " != " << contracted
                      << std::endl;
        }
    }

    const int nb_options = plan.getNbOptions();
    RandomChooser<int> option_chooser(seed);
    for(int i = 0; i < nb_options; ++i) option_chooser.add(i, 1);

    std::default_random_engine gen;
    gen.seed(seed + 1);
    std::uniform_int_distribution<> dis(0, nb_options);

    for(int i = 0; i < 1000; ++i) {
        std::cout << i << "/" << 1000 << std::endl;

        std::vector<int> picked_options;
        double nb_picked_options = dis(gen);
        option_chooser.reset();
        for(int j = 0; j < nb_picked_options; ++j)
            picked_options.push_back(option_chooser.pick());

        DecoredLandscape<MutableLandscape> decored_landscape(landscape);
        for(int option_id : picked_options)
            decored_landscape.apply(nodeOptions[option_id],
                                    arcOptions[option_id]);

        for(MutableLandscape::NodeIt t(graph); t != lemon::INVALID; ++t) {
            const ContractionResult & result = *(*results)[t];

            DecoredLandscape<StaticLandscape> decored_contracted_landscape(
                result.landscape);
            for(int option_id : picked_options)
                decored_contracted_landscape.apply(t_nodeOptions[t][option_id],
                                                   t_arcOptions[t][option_id]);

            const double base = compute_value_reversed(decored_landscape, t);
            const double contracted =
                compute_value_reversed(decored_contracted_landscape, result.t);

            if(fabs(base - contracted) > epsilon) {
                std::cout << graph.id(t) << " : " << base
                          << " != " << contracted << std::endl;
            }
        }
    }

    int sum_of_nb_nodes = 0;
    int sum_of_nb_arcs = 0;
    for(MutableLandscape::NodeIt t(graph); t != lemon::INVALID; ++t) {
        const ContractionResult & result = *(*results)[t];
        sum_of_nb_nodes += lemon::countNodes(result.landscape.getNetwork());
        sum_of_nb_arcs += lemon::countArcs(result.landscape.getNetwork());
    }

    std::cout << "Total nb of nodes : " << sum_of_nb_nodes << std::endl;
    std::cout << "Total nb of arcs : " << sum_of_nb_arcs << std::endl;

    // int normal_time_us =
    // std::chrono::duration_cast<std::chrono::milliseconds>(t1-t0).count();
    // std::cout << "Contraction done in " << normal_time_us << " ms" <<
    // std::endl;

    return EXIT_SUCCESS;
}
