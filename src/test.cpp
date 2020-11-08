#include <iostream>
#include <filesystem>
#include <fstream>

#include "utils/random_chooser.hpp"
#include "indices/eca.hpp"

#include "helper.hpp"

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

int main (int argc, const char *argv[]) {
    if(argc < 2) {
        std::cerr << "input requiered : <landscape_file>" << std::endl;
        return EXIT_FAILURE;
    }

    std::filesystem::path landscape_path = argv[1];

    Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);
    const Graph_t & graph = landscape->getNetwork();

    RestorationPlan<Landscape>plan(*landscape);


    RandomChooser<Graph_t::Arc> arc_chooser;
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a)
        arc_chooser.add(a, 1.0);
    for(int i=0; i<10; i++) {
        Graph_t::Arc a = arc_chooser.pick();
        RestorationPlan<Landscape>::Option* option = plan.addOption();
        option->setId(i);
        option->setCost(1);
        option->addLink(a, landscape->getProbability(a));

        landscape->setProbability(a, std::numeric_limits<double>::epsilon());
    }

    StdLandscapeParser::get().write(*landscape, "output", "probability_test", false);

    StdRestorationPlanParser parser(*landscape);
    parser.write(plan, "output", "probability_test", false);
        
    Helper::assert_well_formed(*landscape, plan);
    delete landscape;

    return EXIT_SUCCESS;
}
