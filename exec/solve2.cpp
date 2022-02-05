/**
 * @file solve.cpp
 * @author François Hamonic (francois.hamonic@gmail.com)
 * @brief Entry program
 * @version 0.1
 * @date 2020-05-07
 *
 * Entry programm that takes inputs and calls solvers.
 */

#include <filesystem>
#include <iostream>

#include <boost/range/algorithm/find_if.hpp>

#include "parsers/std_mutable_landscape_parser.hpp"
#include "parsers/std_restoration_plan_parser.hpp"

#include "solvers/bogo.hpp"
#include "solvers/glutton_eca_dec.hpp"
#include "solvers/glutton_eca_inc.hpp"
#include "solvers/naive_eca_dec.hpp"
#include "solvers/naive_eca_inc.hpp"
#include "solvers/pl_eca_2.hpp"
#include "solvers/pl_eca_3.hpp"
#include "solvers/randomized_rounding.hpp"

#include "helper.hpp"
#include "print_helper.hpp"

#include <boost/range/algorithm.hpp>
namespace br = boost::range;

#include <boost/program_options.hpp>
namespace bpo = boost::program_options;

#include "bpo_utils.hpp"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
namespace logging = boost::log;

void init_logging() {
    logging::core::get()->set_filter(logging::trivial::severity >=
                                     logging::trivial::warning);
}

static bool process_command_line(
    int argc, const char * argv[], std::shared_ptr<concepts::Solver> & solver,
    std::filesystem::path & instances_description_json_file, double & budget,
    bool & output_in_file, std::filesystem::path & output_csv_file) {
    std::vector<std::shared_ptr<concepts::Solver>> solvers{
        std::make_unique<Solvers::Bogo>(),
        std::make_unique<Solvers::Naive_ECA_Inc>(),
        std::make_unique<Solvers::Naive_ECA_Dec>(),
        std::make_unique<Solvers::Glutton_ECA_Inc>(),
        std::make_unique<Solvers::Glutton_ECA_Dec>(),
        std::make_unique<Solvers::PL_ECA_2>(),
        std::make_unique<Solvers::PL_ECA_3>(),
        std::make_unique<Solvers::Randomized_Rounding_ECA>()};

    std::string solver_name;
    std::vector<std::string> solver_params;

    try {
        bpo::options_description desc("Allowed options");
        desc.add_options()("help,h", "produce help message")(
            "solver,s", bpo::value<std::string>(&solver_name)->required(),
            "select the solver to use")("list-solvers,ls",
                                        "list the available solvers")(
            "params,p",
            bpo::value<std::vector<std::string>>(&solver_params)->multitoken(),
            "set the parameters of the selected solver")(
            "list-params",
            "list the available parameters of the selected solver")(
            "instance,i",
            bpo::value<std::filesystem::path>(&instances_description_json_file)
                ->required(),
            "set instance decription json file")(
            "budget,B", bpo::value<double>(&budget)->required(),
            "set the budget value")(
            "output,o", bpo::value<std::filesystem::path>(&output_csv_file),
            "set solution output csv file");

        bpo::variables_map vm;
        bpo::store(bpo::command_line_parser(argc, argv).options(desc).run(),
                   vm);
        if(vm.count("help")) {
            std::cout << desc << "\n";
            return false;
        }
        if(vm.count("list-solvers")) {
            std::cout << "Available solvers:" << std::endl;
            for(auto & s : solvers) std::cout << "\t" << s->name() << std::endl;
            return false;
        }
        bpo::notify(vm);
        auto it = br::find_if(solvers, [&solver_name](auto & s) {
            return s->name() == solver_name;
        });
        if(it == solvers.end())
            throw std::invalid_argument(
                solver_name +
                " is not an available solver, see --list-solvers.");
        solver = *it;
        if(vm.count("list-params")) {
            std::cout << "Available options for " << solver_name << ":"
                      << std::endl;
            for(auto & param : solver->getParams())
                std::cout << "\t" << param.first << std::endl;
            return false;
        }
        for(auto param : solver_params) {
            auto [param_name, param_value] =
                BPOUtils::split_equality_str(param);
            bool param_exists =
                solver->setParam(param_name, param_value.data());
            if(!param_exists) {
                std::cout << "Invalid parameter '" << param << "' for "
                          << solver_name << ", see --list-params." << std::endl;
                return false;
            }
            if(param_value.empty()) {
                 std::cout << "Invalid value for parameter '" << param_name << "' of "
                          << solver_name << std::endl;
                return false;
            }
        }
        output_in_file = vm.count("output");
    } catch(std::exception & e) {
        std::cerr << "Error: " << e.what() << "\n";
        return false;
    }
    return true;
}

int main(int argc, const char * argv[]) {
    std::shared_ptr<concepts::Solver> solver;
    std::filesystem::path instances_description_json;
    double budget;
    bool output_in_file;
    std::filesystem::path output_csv;

    bool valid_command =
        process_command_line(argc, argv, solver, instances_description_json,
                             budget, output_in_file, output_csv);
    if(!valid_command) return EXIT_FAILURE;
    init_logging();

    // Instance2 instance = parse_instance(instances_description_json);

    // Helper::assert_well_formed(instance.landscape, instance.plan);

    // Solution solution = solver.solve(instance.landscape, instance.plan, instance.budget);

    // Helper::printSolution(instance.landscape, instance.plan, name, solver, instance.budget, solution);

    return EXIT_SUCCESS;
}
