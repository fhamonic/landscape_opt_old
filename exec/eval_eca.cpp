#include <iostream>
#include <filesystem>

#include "parsers/std_landscape_parser.hpp"
#include "parsers/std_restoration_plan_parser.hpp"
#include "instances_helper.hpp"

#include "helper.hpp"

int main(int argc, const char *argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <landscape_file>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];

    const Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);   

    // Helper::printLandscape(*landscape, "test.eps");

    const double eca = ECA::get().eval(*landscape);

    std::cout << eca << std::endl;

    delete landscape;

    return EXIT_SUCCESS;
}
