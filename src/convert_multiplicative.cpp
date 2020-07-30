#include <iostream>
#include <filesystem>
#include <fstream>

#include "random_chooser.hpp"
#include "indices/eca.hpp"

#include "helper.hpp"

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

int main (int argc, const char *argv[]) {
    if(argc < 4) {
        std::cerr << "input requiered : <landscape_file> [<plan_file>] <median> <pow>" << std::endl;
        return EXIT_FAILURE;
    }
    bool b_plan = (argc == 5);

    std::filesystem::path landscape_path = argv[1];
    const double median = std::atof(argv[2 + b_plan]);
    const double pow = std::atof(argv[3 + b_plan]);

    const double alpha = -std::pow(median, pow) / std::log(0.5);

    Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);
    const Graph_t & graph = landscape->getNetwork();

    RestorationPlan * plan = nullptr;

    auto p = [&] (const double d) { return std::exp(-std::pow(d,pow)/alpha); };

    if(b_plan) {
        std::filesystem::path plan_path = argv[2];
        StdRestorationPlanParser parser(*landscape);
        plan = parser.parse(plan_path);
        for(RestorationPlan::Option * option : plan->options())
            for(Graph_t::Arc a : option->arcs())
                option->getRestoredProbabilityRef(a) = p(landscape->getProbability(a) - option->getRestoredProbability(a));
        parser.write(*plan, "output", "probability_test", false);
    }

    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a)
        landscape->setProbability(a, p(landscape->getProbability(a)));

    StdLandscapeParser::get().write(*landscape, "output", "probability_test", false);
    
    if(b_plan) {
        Helper::assert_well_formed(*landscape, *plan);
        delete plan;
    }
    delete landscape;

    return EXIT_SUCCESS;
}
