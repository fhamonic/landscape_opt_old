/**
 * @file solve.cpp
 * @author François Hamonic (francois.hamonic@gmail.com)
 * @brief Entry program
 * @version 0.1
 * @date 2020-05-07
 *  
 * Entry programm that takes inputs and calls solvers.
 */

#include <iostream>
#include <filesystem>

#include "parsers/std_landscape_parser.hpp"
#include "parsers/std_restoration_plan_parser.hpp"
#include "instances_helper.hpp"

#include "helper.hpp"

int main() {
    Instance * instance = make_instance_quebec(1, 0.001, 900, Point(0,0), Point(320000000, 320000000));
    // Instance * instance = make_instance_marseillec(1, 0.05, 900, 60);          
    const Landscape & landscape = instance->landscape;      
    const RestorationPlan<Landscape> & plan = instance->plan;

    // Landscape * landscape = StdLandscapeParser::get().parse("/home/plaiseek/Projects/landscape_opt_cpp/data/worst_cases/glutton_eca_dec/dec_worst_case.index");
    
    // StdRestorationPlanParser parser(*landscape);
    // RestorationPlan<Landscape> * plan = parser.parse("/home/plaiseek/Projects/landscape_opt_cpp/data/worst_cases/glutton_eca_dec/dec_worst_case.problem");

    Helper::printInstance(landscape, plan, "test.eps");

    const Graph_t & graph = landscape.getNetwork();
    std::cout << "#nodes " << lemon::countNodes(graph) << std::endl << "#arcs " << lemon::countArcs(graph) << std::endl;

    return EXIT_SUCCESS;
}
