#include "solvers/pl_eca_2.hpp"

#include "gurobi_c++.h"
#include "coin/CoinStructuredModel.hpp"

namespace Solvers::PL_ECA_2_Vars {
    class XVar : public OSI_Builder::VarType {
        private:
            const Graph_t & _graph;
            const int _m;
        public:
            XVar(const Graph_t & graph, int n, int m): VarType(n*m, 0, OSI_Builder::INFTY, false), _graph(graph), _m(m) {}
            int id(Graph_t::Node t, Graph_t::Arc a) { const int id = _graph.id(t) * _m + _graph.id(a);
                assert(id >=0 && id < _number); return _offset + id; }
    };
    class RestoredXVar : public OSI_Builder::VarType {
        private:
            const Graph_t & _graph;
            int _nb_vars_by_node;
            std::vector<int> offsets;
        public:
            RestoredXVar(const RestorationPlan & plan, int n): VarType(0, 0, OSI_Builder::INFTY, false), _graph(plan.getLandscape().getNetwork()) {
                offsets.resize(plan.getNbOptions());
                int cpt = 0;
                for(RestorationPlan::Option * option : plan.options()) { offsets[option->getId()] = cpt; cpt += option->getNbArcs(); }
                _nb_vars_by_node = cpt;
                _number = n * _nb_vars_by_node;
            }
            int id(Graph_t::Node t, Graph_t::Arc a, RestorationPlan::Option * option) { const int id = _graph.id(t) * _nb_vars_by_node + offsets.at(option->getId()) + option->id(a);
                assert(id >=0 && id < _number); return _offset + id;}
    };
    class FVar : public OSI_Builder::VarType {
        private:
            const Graph_t & _graph;
        public:
            FVar(const Graph_t & graph, int n): VarType(n), _graph(graph) {}
            int id(Graph_t::Node u) { const int id = _graph.id(u);
                assert(id >=0 && id < _number); return _offset + id;}
    };
    class RestoredFVar : public OSI_Builder::VarType {
        private:
            const RestorationPlan & _plan;
            std::map<const RestorationPlan::Option *, int> offsets;
        public:
            RestoredFVar(const RestorationPlan & plan): VarType(0, 0, OSI_Builder::INFTY, false), _plan(plan) {
                int cpt = 0;
                for(RestorationPlan::Option * option : plan.options()) { offsets[option] = cpt; cpt += option->getNbNodes(); }
                _number = cpt;
            }
            int id(Graph_t::Node t, const RestorationPlan::Option * option) { const int id = offsets.at(option) + option->id(t);
                assert(id >=0 && id < _number); return _offset + id;}
    };
    class YVar : public OSI_Builder::VarType {
        public:
            YVar(const RestorationPlan & plan): VarType(plan.getNbOptions(), 0, 1, true) {}
            int id(const RestorationPlan::Option * option) { const int id = option->getId();
                assert(id >=0 && id < _number); return _offset + id;}
    };


    void name_variables(OsiClpSolverInterface * solver, const Landscape & landscape, const RestorationPlan & plan, XVar & x_var, RestoredXVar & restored_x_var, FVar & f_var, RestoredFVar & restored_f_var, YVar & y_var) {
        const Graph_t & graph = landscape.getNetwork();
        
        auto node_str = [&graph] (Graph_t::Node v) { return std::to_string(graph.id(v)); };
        auto arc_str = [&graph, &node_str] (Graph_t::Arc a) { return "(" + node_str(graph.source(a)) + "," + node_str(graph.target(a)) + ")"; };

        // XVar
        for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t)
            for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) 
                solver->setColName(x_var.id(t, a), "x_t_" + node_str(t) + "_a_" + arc_str(a));
        // RestoredXVar
        for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t)
            for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) 
                for(RestorationPlan::Option * option : plan.getOptions(a))
                    solver->setColName(restored_x_var.id(t, a, option), "restored_x_t_" + node_str(t) + "_a_" + arc_str(a) + "_" + std::to_string(option->getId()));
        // FVar
        for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) solver->setColName(f_var.id(t), "f_t_" +  node_str(t));
        // RestoredFVar
        for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t)
            for(RestorationPlan::Option * option : plan.getOptions(t))
                solver->setColName(restored_f_var.id(t, option), "restored_f_t_" + node_str(t) + "_" + std::to_string(option->getId()));
        // YVar
        for(RestorationPlan::Option * option : plan.options()) solver->setColName(y_var.id(option), "y_" + std::to_string(option->getId()));
    }
}

using namespace Solvers::PL_ECA_2_Vars;

Solution * Solvers::PL_ECA_2::solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const int nb_threads = params.at("threads")->getInt();
    const bool relaxed = params.at("relaxed")->getBool();
    const int timeout = params.at("timeout")->getInt();
    //const bool fortest = params.at("fortest")->getBool();
    std::chrono::time_point<std::chrono::high_resolution_clock> last_time, current_time;

    const Graph_t & graph = landscape.getNetwork();

    const int n = lemon::countNodes(graph);
    const int m = lemon::countArcs(graph);

    XVar x_var(graph, n, m);
    RestoredXVar restored_x_var(plan, n);
    FVar f_var(graph, n);
    RestoredFVar restored_f_var(plan);
    YVar y_var(plan);

    double sum = 0;
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) { 
        sum += landscape.getQuality(v);
        for(RestorationPlan::Option * option : plan.getOptions(v)) 
            sum += option->getQualityGain(v);
    }
    const double M = sum;

    OSI_Builder solver_builder = OSI_Builder();

    solver_builder.addVarType(&x_var);
    solver_builder.addVarType(&restored_x_var);
    solver_builder.addVarType(&f_var);
    solver_builder.addVarType(&restored_f_var);
    solver_builder.addVarType(&y_var);
    solver_builder.init();
    
    if(log_level > 0)
        std::cout << name() << ": Start filling solver : " << solver_builder.getNbVars() << " variables" << std::endl;
    last_time = std::chrono::high_resolution_clock::now();

    ////////////////////////////////////////////////////////////////////////
    // Columns : Objective
    ////////////////////
    for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
        // sum w(t) * f_t
        const int f_t = f_var.id(t);
        solver_builder.setObjective(f_t, landscape.getQuality(t));
        for(RestorationPlan::Option * option : plan.getOptions(t)) {   
            const int restored_f_t = restored_f_var.id(t, option);
            solver_builder.setObjective(restored_f_t, option->getQualityGain(t));
        }
    }
    ////////////////////////////////////////////////////////////////////////
    // Rows : Constraints
    ////////////////////
    for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
        const int f_t = f_var.id(t);
        // out_flow(u) - in_flow(u) <= w(u)
        for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) {
            // out flow
            for(Graph_t::OutArcIt b(graph, u); b != lemon::INVALID; ++b) {
                const int x_tb = x_var.id(t,b);
                solver_builder.buffEntry(x_tb, 1);
                for(RestorationPlan::Option * option : plan.getOptions(b)) {
                    const int restored_x_t_b = restored_x_var.id(t, b, option);
                    solver_builder.buffEntry(restored_x_t_b, 1);
                }
            }
            // in flow
            for(Graph_t::InArcIt a(graph, u); a != lemon::INVALID; ++a) {
                const int x_ta = x_var.id(t,a);
                solver_builder.buffEntry(x_ta, -landscape.getProbability(a));
                for(RestorationPlan::Option * option : plan.getOptions(a)) {
                    const int restored_x_t_a = restored_x_var.id(t, a, option);
                    // Warning Length Gains are compose by max  (in place of + intuitively)
                    solver_builder.buffEntry(restored_x_t_a, -option->getRestoredProbability(a));
                }
            }
            // optional injected flow
            for(RestorationPlan::Option * option : plan.getOptions(u)) {
                const int y_u = y_var.id(option);
                solver_builder.buffEntry(y_u, -option->getQualityGain(u));
            }
            // optimisation variable
            if(u == t)
                solver_builder.buffEntry(f_t, 1);
            // injected flow
            solver_builder.pushRow(landscape.getQuality(u), landscape.getQuality(u));
        }

        // x_a < y_i * M
        for(RestorationPlan::Option * option : plan.options()) {
            const int y_i = y_var.id(option);
            for(Graph_t::Arc a : option->arcs()) {
                const int x_ta = restored_x_var.id(t, a, option);
                solver_builder.buffEntry(y_i, M);
                solver_builder.buffEntry(x_ta, -1);
                solver_builder.pushRow(0, OSI_Builder::INFTY);
            }
        }
    }
    // restored_f_t <= f_t
    // restored_f_t <= y_i * M
    for(RestorationPlan::Option * option : plan.options()) {
        const int y_i = y_var.id(option);
        for(Graph_t::Node t : option->nodes()) {
            const int f_t = f_var.id(t);
            const int restored_f_t = restored_f_var.id(t, option);
            solver_builder.buffEntry(f_t, 1);
            solver_builder.buffEntry(restored_f_t, -1);
            solver_builder.pushRow(0, OSI_Builder::INFTY);

            solver_builder.buffEntry(y_i, M);
            solver_builder.buffEntry(restored_f_t, -1);
            solver_builder.pushRow(0, OSI_Builder::INFTY);
        }
    }
    ////////////////////
    // sum y_i < B
    for(RestorationPlan::Option * option : plan.options()) {
        const int y_i = y_var.id(option);
        solver_builder.buffEntry(y_i, option->getCost());
    }
    solver_builder.pushRow(0, B);
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    
    current_time = std::chrono::high_resolution_clock::now();
    if(log_level >= 1) {
        int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-last_time).count();
        std::cout << name() << ": Complete filling solver : " << solver_builder.getNbConstraints() << " constraints in " << time_ms << " ms" << std::endl;
        std::cout << name() << ": Start solving" << std::endl;
    }
    last_time = current_time;

    OsiClpSolverInterface * solver = solver_builder.buildSolver(OSI_Builder::MAX);

    if(!relaxed) {
        for(RestorationPlan::Option * option : plan.options()) {
            const int y_i = y_var.id(option);
            solver->setInteger(y_i);
        }
    }

    if(log_level >= 1) {
        name_variables(solver, landscape, plan, x_var, restored_x_var, f_var, restored_f_var, y_var);
        solver->writeLp("pl_eca_2");
        solver->writeMps("pl_eca_2");
    }

    CbcModel model(*solver);

    model.setIntegerTolerance(1.0e-14);
    model.setAllowableGap(1.0e-14);

    model.setLogLevel(log_level >= 2 ? 1 : 0);

    if(model.haveMultiThreadSupport())  {
        model.setNumberThreads(nb_threads);
        if(log_level >= 1)
            std::cout << name() << ": Enabled multithread : " << model.getNumberThreads() << std::endl;
    }
    else 
        if(log_level >= 1)
            std::cerr << name() << ": multithread is disabled : to enable it build Cbc with --enable-cbc-parallel" << std::endl;
          
    ////////////////////////////////////////////////////////////////////////
    // Compute
    ////////////////////
    model.setMaximumSeconds(timeout);
    model.branchAndBound();
    
    const double * var_solution = model.bestSolution();
    if(var_solution == nullptr) {
        std::cerr << name() << ": Fail" << std::endl;
        return nullptr;
    }

    Solution * solution = new Solution(landscape, plan);

    for(RestorationPlan::Option * option : plan.options()) {
        const int y_i = y_var.id(option);
        const double value = var_solution[y_i];
        solution->set(option, value);
    }


    current_time = std::chrono::high_resolution_clock::now();
    int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-last_time).count();
    solution->setComputeTimeMs(time_ms);
    last_time = current_time;

    if(log_level >= 1) {
        std::cout << name() << ": Complete solving : " << time_ms << " ms" << std::endl;
        std::cout << name() << ": ECA from obj : " << std::sqrt(model.getObjValue()) << std::endl;
        std::cout << name() << ": Explored nodes : " << model.getNodeCount() << std::endl;
    }
    
    solution->obj = std::sqrt(model.getObjValue());
    solution->nb_vars = solver_builder.getNbVars();
    solution->nb_constraints = solver_builder.getNbConstraints();

    delete solver;

    return solution;
}