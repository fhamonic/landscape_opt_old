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

static bool process_command_line(int argc, char * argv[],
                                 std::shared_ptr<concepts::Solver> & solver,
                                 std::filesystem::path & instances_description_json,
                                 double & budget,
                                 bool & output_in_file,
                                 std::filesystem::path & output_csv) {
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
            bpo::value<vector<std::string>>(&solver_params)->multitoken(),
            "set the parameters of the selected solver")(
            "list-params",
            "list the available parameters of the selected solver")(
            "instance,i",
            bpo::value<std::filesystem::path>(&instances_description_json_file)
                ->required(),
            "set instance decription json file")("budget,B",
                                   bpo::value<double>(budget)->required(),
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
            for(auto & p : solvers) std::cout << "\t" << p.first << std::endl;
            return false;
        }
        bpo::notify(vm);
        auto it = br::find_if(solvers, [&solver_name](auto & s) {
            return s.name() == solver_name;
        });
        if(it == solvers.end())
            throw std::invalid_argument(
                solver_name +
                " is not an available solver, see --list-solvers.");
        solver = it->second;
        if(vm.count("list-params")) {
            std::cout << "Available options for " << solver_name << ":"
                      << std::endl;
            for(auto & param : solver->getParamList())
                std::cout << "\t" << param << std::endl;
            return false;
        }
        for(auto param : solver_params) {
            auto [param_name, param_value] = BPOUtils::split_equality_str(param);
            bool param_exists = solver->setParam(param_name, param_value.data());
            if(!param_exists) {
               std::cout << "Invalid parameter '"<< param <<"' for " << solver_name << ", see:"
                      << std::endl;
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

    bool valid_command = process_command_line(
        argc, argv, solver, instances_description_json, budget, output_in_file, output_csv);
    if(!valid_command) return EXIT_FAILURE;
    init_logging();

    std::filesystem::path landscape_path = argv[1];
    std::filesystem::path problem_path = argv[2];
    double B = std::atof(argv[3]);
    std::string solver_name = argv[4];

    auto it = boost::find_if(solvers, [&solver_name](const auto & solver) {
        return solver->name() == solver_name;
    });

    if(it == solvers.end()) {
        std::cerr << "Availables solvers :" << std::endl;
        for(const auto & solver : solvers)
            std::cerr << "\t" << solver->name() << std::endl;
        return EXIT_FAILURE;
    }

    concepts::Solver & solver = std::ref(*it->get());

    for(int i = 5; i < argc; i++) {
        std::string arg = argv[i];
        const int split_index = arg.find("=");
        std::string option = arg.substr(0, split_index);
        std::string value = arg.substr(split_index + 1, -1);

        if(!solver.setParam(option, value.data())) {
            auto params = solver.getParams();
            if(params.empty()) {
                std::cerr << "No options available for \"" << solver.name()
                          << "\"" << std::endl;
            } else {
                std::cerr << "Available options for \"" << solver.name()
                          << "\" :" << std::endl;
                for(const auto & [param_name, param_value] : params)
                    std::cerr << "\t" << param_name << std::endl;
            }
            return EXIT_FAILURE;
        }
    }

    std::string name = std::filesystem::path(problem_path).stem();

    MutableLandscape landscape =
        StdMutableLandscapeParser::get().parse(landscape_path);

    StdRestorationPlanParser parser(landscape);
    RestorationPlan<MutableLandscape> plan = parser.parse(problem_path);

    // Helper::assert_well_formed(landscape, plan);

    plan.initElementIDs();
    Solution solution = solver.solve(landscape, plan, B);

    Helper::printSolution(landscape, plan, name, solver, B, solution);

    return EXIT_SUCCESS;
}
