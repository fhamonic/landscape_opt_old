#include <iostream>
#include <filesystem>
#include <fstream>

#include "random_chooser.hpp"
#include "indices/eca.hpp"

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

int main (int argc, const char *argv[]) {
    if(argc < 4) {
        std::cerr << "input requiered : <landscape_file> <plan_file> <alpha>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];
    std::filesystem::path plan_path = argv[2];
    const double alpha = std::atof(argv[3]);

    Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);
    const Graph_t & graph = landscape->getNetwork();

    StdRestorationPlanParser parser(*landscape);
    RestorationPlan * plan = parser.parse(plan_path);
    
    for(RestorationPlan::Option * option : plan->options())
        for(Graph_t::Arc a : option->arcs())
            option->getRestoredProbabilityRef(a) = ECA::P_func( landscape->getProbability(a) - option->getRestoredProbability(a) , alpha);

    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a)
        landscape->setProbability(a, ECA::P_func( landscape->getProbability(a) , alpha ));


    StdLandscapeParser::get().write(*landscape, "output", "probability_test", false);
    parser.write(*plan, "output", "probability_test", false);
    

    return EXIT_SUCCESS;
}
