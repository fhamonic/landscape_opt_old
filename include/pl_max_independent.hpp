#ifndef PL_MAX_INDEPENDENT_HPP
#define PL_MAX_INDEPENDENT_HPP

#include "landscape.hpp"
#include "osi_builder.hpp"

#include "coin/CbcModel.hpp"

Graph_t::NodeMap<bool> * getMaxIndependentMap(const Graph_t & graph) {
    bool relaxed = false;
    int log_level = 0;
    int nb_threads = 6;

    const int n = lemon::countNodes(graph);

    auto x_var = [&] (Graph_t::Node v) {
       return graph.id(v); 
    };
    const int x_var_number = n;

    OSI_Builder * solver_builder = new OSI_Builder();

    solver_builder->addVarType(x_var_number, NULL, NULL, 0, 1);
    solver_builder->init();

    ////////// Objective ///////////
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        const int x_v = x_var(v);
        solver_builder->setObjective(x_v, 1);
    }
    ///////// Constraints /////////
    for(Graph_t::ArcIt e(graph); e != lemon::INVALID; ++e) {
        const int x_u = x_var(graph.u(e));
        const int x_v = x_var(graph.v(e));

        solver_builder->buffEntry(x_u, 1);
        solver_builder->buffEntry(x_v, 1);
        solver_builder->pushRow(0, 1);
    }

    OsiClpSolverInterface * solver = solver_builder->buildSolver(OsiClpSolver_Builder::MAX);

    if(!relaxed) {
        for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
            const int x_v = x_var(v);
            solver->setInteger(x_v);
        }
    }

    CbcModel model(*solver);
    model.setLogLevel(log_level);
    if(nb_threads > 1) {
        if(model.haveMultiThreadSupport()) 
            model.setNumberThreads(nb_threads);
        else
            std::cout << "multithreaded is disabled : to enable it build Cbc with --enable-cbc-parallel" << std::endl;
    }    
 
    model.branchAndBound();
    
    const double * var_solution = model.bestSolution();
    if(var_solution == NULL) {
        std::cerr << "Fail" << std::endl;
        return NULL;
    }

    Graph_t::NodeMap<bool> * independentMap = new Graph_t::NodeMap<bool>(graph);
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        const int x_v = x_var(v);
        (*independentMap)[v] = (var_solution[x_v] == 1.0);
    }
    return independentMap;
}


#endif