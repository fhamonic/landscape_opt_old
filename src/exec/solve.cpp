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

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

#include "solvers/bogo.hpp"
#include "solvers/naive_eca_inc.hpp"
#include "solvers/naive_eca_dec.hpp"
#include "solvers/glutton_eca_inc.hpp"
#include "solvers/glutton_eca_dec.hpp"
#include "solvers/pl_eca_2.hpp"
#include "solvers/pl_eca_3.hpp"
#include "solvers/randomized_rounding.hpp"

#include "helper.hpp"

/**
 * @brief Instanciates solvers
 * 
 * Instanciates the solvers, providing default options, and fill the required data structures for parsing.
 * 
 * @param solvers reference to the list of instanciated solvers
 * @param solversMap reference to the map associating a solvers name to its instance
 */
static void populate(std::list<concepts::Solver*> & solvers, std::map<std::string, concepts::Solver*> & solversMap) {
    Solvers::Bogo * bogo = new Solvers::Bogo();
    Solvers::Naive_ECA_Inc * naive_eca_inc = new Solvers::Naive_ECA_Inc();
    Solvers::Naive_ECA_Dec * naive_eca_dec = new Solvers::Naive_ECA_Dec();
    Solvers::Glutton_ECA_Inc * glutton_eca_inc = new Solvers::Glutton_ECA_Inc();
    Solvers::Glutton_ECA_Dec * glutton_eca_dec = new Solvers::Glutton_ECA_Dec();
    Solvers::PL_ECA_2 * pl_eca_2 = new Solvers::PL_ECA_2();
    (*pl_eca_2).setLogLevel(0);
    Solvers::PL_ECA_3 * pl_eca_3 = new Solvers::PL_ECA_3();
    (*pl_eca_3).setLogLevel(0);
    Solvers::Randomized_Rounding_ECA * randomized_rounding = new Solvers::Randomized_Rounding_ECA();
    randomized_rounding->setLogLevel(0).setNbDraws(100);

    solvers.push_back(bogo);
    solvers.push_back(naive_eca_inc);
    solvers.push_back(naive_eca_dec);
    solvers.push_back(glutton_eca_inc);
    solvers.push_back(glutton_eca_dec);
    solvers.push_back(pl_eca_2);
    solvers.push_back(pl_eca_3);
    solvers.push_back(randomized_rounding);

    for(concepts::Solver * solver : solvers) {
        solversMap[solver->name()] = solver;
    }
}
/**
 * @brief Deletes the instanciated solvers
 * 
 * @param solvers reference to the list of instanciated solvers
 */
static void clean(std::list<concepts::Solver*> & solvers) {
    for(concepts::Solver * solver : solvers)
        delete solver;
}

int main(int argc, const char *argv[]) {
    std::cout.precision(10);

    std::list<concepts::Solver*> solvers;
    std::map<std::string, concepts::Solver*> solversMap;
    populate(solvers, solversMap);
    if(argc < 5) {
        std::cerr << "input requiered : <landscape_file> <problem_file> <B> <solver_name> [<option>=<value>]" << std::endl;
        clean(solvers);
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];
    std::filesystem::path problem_path = argv[2];
    double B = std::atof(argv[3]);
    std::string solver_name = argv[4];

    if(!solversMap.contains(solver_name)) {
        std::cerr << "Availables solvers :" << std::endl;
        for(concepts::Solver * solver : solvers)
            std::cerr << "\t" << solver->name() << std::endl;
        clean(solvers);
        return EXIT_FAILURE;
    }

    concepts::Solver & solver = *solversMap[solver_name];

    for(int i=5; i<argc; i++) {
        std::string arg = argv[i];
        const int split_index = arg.find("=");
        std::string option = arg.substr(0, split_index);
        std::string value = arg.substr(split_index+1, -1);

        if(!solver.setParam(option, value.data())) {
            std::list<std::string> * paramList = solver.getParamList();
            if(paramList->size() == 0) {
                std::cerr << "No options available for \"" << solver.name() << "\"" << std::endl;
            } else {
                std::cerr << "Available options for \"" << solver.name() << "\" :" << std::endl;
                for(std::string option_name : *paramList)
                    std::cerr << "\t" << option_name << std::endl;
            }            
            delete paramList;
            clean(solvers);
            return EXIT_FAILURE;
        }       
    }

    std::string name = std::filesystem::path(problem_path).stem();

    Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);

    StdRestorationPlanParser parser(*landscape);
    RestorationPlan<Landscape>* plan = parser.parse(problem_path);

    //for debug purposes
    //Helper::assert_well_formed(*landscape, *plan);

    Solution * solution = solver.solve(*landscape, *plan, B);

    if(solution != nullptr) {
        Helper::printSolution(*landscape, *plan, name, solver, B, solution);
        delete solution;
    } else {
        std::cerr << "Fail" << std::endl;
    }

    delete plan;
    delete landscape;
    clean(solvers);

    return EXIT_SUCCESS;
}
