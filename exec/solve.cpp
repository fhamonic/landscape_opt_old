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

#include <boost/range/algorithm/find_if.hpp>

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

#include "solvers/bogo.hpp"
#include "solvers/naive_eca_inc.hpp"
#include "solvers/naive_eca_dec.hpp"
#include "solvers/glutton_eca_inc.hpp"
#include "solvers/glutton_eca_dec.hpp"
#include "solvers/pl_eca_2.hpp"
#include "solvers/pl_eca_3.hpp"
// #include "solvers/pl_eca_4.hpp"
#include "solvers/randomized_rounding.hpp"

#include "helper.hpp"
#include "print_helper.hpp"

/**
 * @brief Instanciates solvers
 * 
 * Instanciates the solvers, providing default options, and fill the required data structures for parsing.
 * 
 * @param solvers reference to the list of instanciated solvers
 * @param solversMap reference to the map associating a solvers name to its instance
 */
static std::vector<std::unique_ptr<concepts::Solver>> construct_solvers() {
    std::vector<std::unique_ptr<concepts::Solver>> solvers;

    solvers.emplace_back(std::make_unique<Solvers::Bogo>());
    solvers.emplace_back(std::make_unique<Solvers::Naive_ECA_Inc>());
    solvers.emplace_back(std::make_unique<Solvers::Naive_ECA_Dec>());
    solvers.emplace_back(std::make_unique<Solvers::Glutton_ECA_Inc>());
    solvers.emplace_back(std::make_unique<Solvers::Glutton_ECA_Dec>());
    solvers.emplace_back(std::make_unique<Solvers::PL_ECA_2>());
    solvers.emplace_back(std::make_unique<Solvers::PL_ECA_3>());
    // solvers.emplace_back(std::make_unique<Solvers::PL_ECA_4>());
    solvers.emplace_back(std::make_unique<Solvers::Randomized_Rounding_ECA>());

    return solvers;
}

void print_usage(const char * prg_name) {
    std::cerr << "Usage: " << prg_name << " <landscape_file> <problem_file> <budget_value> <solver_name> [<option>=<value>]" << std::endl;
}

int main(int argc, const char *argv[]) {
    std::cout.precision(10);

    std::vector<std::unique_ptr<concepts::Solver>> solvers = construct_solvers();
    if(argc < 5) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }
    std::filesystem::path landscape_path = argv[1];
    std::filesystem::path problem_path = argv[2];
    double B = std::atof(argv[3]);
    std::string solver_name = argv[4];

    auto it = boost::find_if(solvers, [&solver_name](const auto & solver){ return solver->name() == solver_name; });

    if(it == solvers.end()) {
        std::cerr << "Availables solvers :" << std::endl;
        for(const auto & solver : solvers)
            std::cerr << "\t" << solver->name() << std::endl;
        return EXIT_FAILURE;
    }

    concepts::Solver & solver = std::ref(*it->get());

    for(int i=5; i<argc; i++) {
        std::string arg = argv[i];
        const int split_index = arg.find("=");
        std::string option = arg.substr(0, split_index);
        std::string value = arg.substr(split_index+1, -1);

        if(!solver.setParam(option, value.data())) {
            auto params = solver.getParams();
            if(params.empty()) {
                std::cerr << "No options available for \"" << solver.name() << "\"" << std::endl;
            } else {
                std::cerr << "Available options for \"" << solver.name() << "\" :" << std::endl;
                for(const auto & [param_name, param_value] : params)
                    std::cerr << "\t" << param_name << std::endl;
            }            
            return EXIT_FAILURE;
        }       
    }

    std::string name = std::filesystem::path(problem_path).stem();

    Landscape landscape = StdLandscapeParser::get().parse(landscape_path);

    StdRestorationPlanParser parser(landscape);
    RestorationPlan<Landscape> plan = parser.parse(problem_path);

    //Helper::assert_well_formed(landscape, plan);

    plan.initElementIDs();
    Solution solution = solver.solve(landscape, plan, B);

    Helper::printSolution(landscape, plan, name, solver, B, solution);

    return EXIT_SUCCESS;
}
