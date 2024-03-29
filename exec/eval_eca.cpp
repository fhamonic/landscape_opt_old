#include <filesystem>
#include <iostream>

#include "instances_helper.hpp"
#include "parsers/std_mutable_landscape_parser.hpp"
#include "parsers/std_restoration_plan_parser.hpp"

#include "helper.hpp"
#include "print_helper.hpp"

int main(int argc, const char * argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <landscape_file>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];

    const MutableLandscape landscape =
        StdMutableLandscapeParser::get().parse(landscape_path);

    Helper::printLandscapeGraphviz(landscape, "test.dot");

    const double eca = ECA().eval(landscape);

    std::cout << eca << std::endl;

    return EXIT_SUCCESS;
}
