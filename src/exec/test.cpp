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
#include "instances_helper.hpp"

#include "helper.hpp"

int main() {
    // Instance * instance = make_instance_quebec(1, 0.01, 900, Point(242513,4987733), Point(32000, 20000), false);
    Instance * instance = make_instance_marseillec(1, 0.05, 900, 60);
                        
    const Landscape & landscape = instance->landscape;

    Helper::printLandscape(landscape, "test.eps");

    return EXIT_SUCCESS;
}
