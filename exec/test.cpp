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
#include "algorithms/multiplicative_dijkstra.hpp"

#include "helper.hpp"
#include "print_helper.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

#include "gcg/solver_mip.h"

int main() {
    Landscape landscape = StdLandscapeParser::get().parse("data/graphe_yann/graphe_yann.index");
    const Graph_t & graph = landscape.getNetwork();

    StdRestorationPlanParser plan_parser(landscape);
    RestorationPlan<Landscape> plan = plan_parser.parse("data/graphe_yann/test.problem");

    Helper::printInstance(landscape, plan, "output/original");

    MyContractionAlgorithm alg2;
    Graph_t::NodeMap<ContractionResult> * contracted_results = alg2.precompute(landscape, plan);

    const ContractionResult & contracted_result = (*contracted_results)[graph.nodeFromId(16)];

    Helper::printInstance(*contracted_result.landscape.get(), *contracted_result.plan.get(), "output/contracted");

    delete contracted_results;

    return EXIT_SUCCESS;
}