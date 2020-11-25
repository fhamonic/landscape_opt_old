#include "solvers/pl_eca_2.hpp"

namespace Solvers::PL_ECA_2_Vars {
    class PreprocessedDatas {
        public:
            std::vector<Graph_t::Node> target_nodes;
            Graph_t::NodeMap<int> target_ids_Map;
            Graph_t::NodeMap<double> M_Map;
            int m;

            PreprocessedDatas(const Landscape & landscape, const RestorationPlan<Landscape>& plan) : target_ids_Map(landscape.getNetwork(), -1), 
                    M_Map(landscape.getNetwork()) {
                const Graph_t & graph = landscape.getNetwork();
                // target_nodes
                for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) { 
                    if(landscape.getQuality(u) == 0 && !plan.contains(u)) continue;
                    target_nodes.push_back(u);
                    target_ids_Map[u] = target_nodes.size()-1;
                }
                // M_Map
                std::for_each(std::execution::par, target_nodes.begin(), target_nodes.end(), [&] (Graph_t::Node t) {
                    M_Map[t] = max_flow_in(landscape, plan, t);
                });

                m = lemon::countArcs(graph);
            }
            ~PreprocessedDatas() {}
    };


    class XVar : public OSI_Builder::VarType {
        private:
            const PreprocessedDatas & _pdatas;
            const Graph_t & _graph;
        public:
            XVar(const PreprocessedDatas & pdatas, const Graph_t & graph): 
                    VarType(pdatas.target_nodes.size()*pdatas.m, 0, OSI_Builder::INFTY, false),
                    _pdatas(pdatas), _graph(graph) {}
            int id(Graph_t::Node t, Graph_t::Arc a) { const int id = _pdatas.target_ids_Map[t] * _pdatas.m + _graph.id(a);
                assert(id >=0 && id < _number); return _offset + id; }
    };
    class RestoredXVar : public OSI_Builder::VarType {
        private:
            const PreprocessedDatas & _pdatas;
            const RestorationPlan<Landscape>& _plan;
            int _nb_vars_by_node;
            std::vector<int> offsets;
        public:
            RestoredXVar(const PreprocessedDatas & pdatas, const RestorationPlan<Landscape>& plan): VarType(0, 0, OSI_Builder::INFTY, false), 
                    _pdatas(pdatas), _plan(plan) {
                int cpt = 0;
                for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i)
                { offsets.push_back(cpt); cpt += plan.getNbArcs(i); }
                _nb_vars_by_node = cpt;
                _number = _pdatas.target_nodes.size() * _nb_vars_by_node;
            }
            int id(Graph_t::Node t, Graph_t::Arc a, RestorationPlan<Landscape>::Option option) { 
                const int id = _pdatas.target_ids_Map[t] * _nb_vars_by_node + offsets.at(option) + _plan.id(option, a);
                assert(id >=0 && id < _number); return _offset + id;}
    };
    class FVar : public OSI_Builder::VarType {
        private:
            const PreprocessedDatas & _pdatas;
        public:
            FVar(const PreprocessedDatas & pdatas): VarType(pdatas.target_nodes.size()), _pdatas(pdatas) {}
            int id(Graph_t::Node u) { const int id = _pdatas.target_ids_Map[u];
                assert(id >=0 && id < _number); return _offset + id;}
    };
    class RestoredFVar : public OSI_Builder::VarType {
        private:
            const RestorationPlan<Landscape>& _plan;
            std::vector<int> offsets;
        public:
            RestoredFVar(const RestorationPlan<Landscape>& plan): VarType(0, 0, OSI_Builder::INFTY, false), _plan(plan) {
                int cpt = 0;
                for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i)
                { offsets.push_back(cpt); cpt += plan.getNbNodes(i); }
                _number = cpt;
            }
            int id(Graph_t::Node t, RestorationPlan<Landscape>::Option i) { 
                const int id = offsets.at(i) + _plan.id(i, t);
                assert(id >=0 && id < _number); return _offset + id;}
    };
    class YVar : public OSI_Builder::VarType {
        public:
            YVar(const RestorationPlan<Landscape>& plan): VarType(plan.getNbOptions(), 0, 1, true) {}
            int id(RestorationPlan<Landscape>::Option option) { const int id = option;
                assert(id >=0 && id < _number); return _offset + id;}
    };


    class Variables {
        private:
            const Graph_t & graph;
        public:
            XVar x;
            RestoredXVar restored_x;
            FVar f;
            RestoredFVar restored_f;
            YVar y;

            Variables(const Landscape & landscape, const RestorationPlan<Landscape>& plan, const PreprocessedDatas & pdatas) : graph(landscape.getNetwork()),
                    x(pdatas, graph), restored_x(pdatas, plan), f(pdatas), restored_f(plan), y(plan) {}
    };


    void name_variables(OSI_Builder & solver, const Landscape & landscape, const RestorationPlan<Landscape>& plan, Variables & vars, const PreprocessedDatas & pdatas) {
        const Graph_t & graph = landscape.getNetwork();
        
        auto node_str = [&graph] (Graph_t::Node v) { return std::to_string(graph.id(v)); };
        auto arc_str = [&graph, &node_str] (Graph_t::Arc a) { return std::to_string(graph.id(a)) + "(" + node_str(graph.source(a)) + "," + node_str(graph.target(a)) + ")"; };

        // XVar
        for(Graph_t::Node t : pdatas.target_nodes)
            for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) 
                solver.setColName(vars.x.id(t, a), "x_t_" + node_str(t) + "_a_" + arc_str(a));
        // RestoredXVar
        for(Graph_t::Node t : pdatas.target_nodes)
            for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) 
                for(auto const& [option, quality_gain] : plan.getOptions(a))
                    solver.setColName(vars.restored_x.id(t, a, option), "restored_x_t_" + node_str(t) + "_a_" + arc_str(a) + "_" + std::to_string(option));
        // FVar
        for(Graph_t::Node t : pdatas.target_nodes) solver.setColName(vars.f.id(t), "f_t_" +  node_str(t));
        // RestoredFVar
        for(Graph_t::Node t : pdatas.target_nodes)
            for(auto const& [option, quality_gain] : plan.getOptions(t))
                solver.setColName(vars.restored_f.id(t, option), "restored_f_t_" + node_str(t) + "_" + std::to_string(option));
        // YVar
        for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i) solver.setColName(vars.y.id(i), "y_" + std::to_string(i));
    }
}

using namespace Solvers::PL_ECA_2_Vars;

void insert_variables(OSI_Builder & solver_builder, Variables & vars) {
    solver_builder.addVarType(&vars.x);
    solver_builder.addVarType(&vars.restored_x);
    solver_builder.addVarType(&vars.f);
    solver_builder.addVarType(&vars.restored_f);
    solver_builder.addVarType(&vars.y);
    solver_builder.init();
}

void fill_solver(OSI_Builder & solver_builder, const Landscape & landscape, const RestorationPlan<Landscape>& plan, const double B, Variables & vars, bool relaxed, PreprocessedDatas & pdatas) {

    const Graph_t & graph = landscape.getNetwork();
    const Graph_t::NodeMap<double> & M = pdatas.M_Map;

    ////////////////////////////////////////////////////////////////////////
    // Columns : Objective
    ////////////////////
    for(Graph_t::Node t : pdatas.target_nodes) {
        if(landscape.getQuality(t) == 0 && !plan.contains(t)) continue;
        // sum w(t) * f_t
        const int f_t = vars.f.id(t);
        solver_builder.setObjective(f_t, landscape.getQuality(t));
        for(auto const& [option, quality_gain] : plan.getOptions(t)) {   
            const int restored_f_t = vars.restored_f.id(t, option);
            solver_builder.setObjective(restored_f_t, quality_gain);
        }
    }
    ////////////////////////////////////////////////////////////////////////
    // Rows : Constraints
    ////////////////////
    for(Graph_t::Node t : pdatas.target_nodes) {
        const int f_t = vars.f.id(t);
        // out_flow(u) - in_flow(u) <= w(u)
        for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) {
            // out flow
            for(Graph_t::OutArcIt b(graph, u); b != lemon::INVALID; ++b) {
                const int x_tb = vars.x.id(t,b);
                solver_builder.buffEntry(x_tb, 1);
                for(auto const& [option, restored_probability] : plan.getOptions(b)) {
                    const int restored_x_t_b = vars.restored_x.id(t, b, option);
                    solver_builder.buffEntry(restored_x_t_b, 1);
                }
            }
            // in flow
            for(Graph_t::InArcIt a(graph, u); a != lemon::INVALID; ++a) {
                const int x_ta = vars.x.id(t,a);
                solver_builder.buffEntry(x_ta, -landscape.getProbability(a));
                for(auto const& [option, restored_probability] : plan.getOptions(a)) {
                    const int restored_x_t_a = vars.restored_x.id(t, a, option);
                    solver_builder.buffEntry(restored_x_t_a, -restored_probability);
                }
            }
            // optional injected flow
            for(auto const& [option, quality_gain] : plan.getOptions(u)) {
                const int y_u = vars.y.id(option);
                solver_builder.buffEntry(y_u, -quality_gain);
            }
            // optimisation variable
            if(u == t)
                solver_builder.buffEntry(f_t, 1);
            // injected flow
            solver_builder.pushRow(-OSI_Builder::INFTY, landscape.getQuality(u));
        }

        // x_a < y_i * M
        for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i) {
            const int y_i = vars.y.id(i);
            for(auto const& [a, restored_probability] : plan.getArcs(i)) {
                const int x_ta = vars.restored_x.id(t, a, i);
                solver_builder.buffEntry(y_i, M[graph.source(a)]);
                solver_builder.buffEntry(x_ta, -1);
                solver_builder.pushRow(0, OSI_Builder::INFTY);
            }
        }
    }
    // restored_f_t <= f_t
    // restored_f_t <= y_i * M
    for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i) {
        const int y_i = vars.y.id(i);
        for(auto const& [t, quality_gain] : plan.getNodes(i)) {
            const int f_t = vars.f.id(t);
            const int restored_f_t = vars.restored_f.id(t, i);
            solver_builder.buffEntry(f_t, 1);
            solver_builder.buffEntry(restored_f_t, -1);
            solver_builder.pushRow(0, OSI_Builder::INFTY);

            solver_builder.buffEntry(y_i, M[t]);
            solver_builder.buffEntry(restored_f_t, -1);
            solver_builder.pushRow(0, OSI_Builder::INFTY);
        }
    }
    ////////////////////
    // sum y_i < B
    for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i) {
        const int y_i = vars.y.id(i);
        solver_builder.buffEntry(y_i, plan.getCost(i));
    }
    solver_builder.pushRow(0, B);
    ////////////////////
    // integer constraints
    if(!relaxed) {
        for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i) {
            const int y_i = vars.y.id(i);
            solver_builder.setInteger(y_i);
        }
    }
}

Solution * Solvers::PL_ECA_2::solve(const Landscape & landscape, const RestorationPlan<Landscape>& plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const bool relaxed = params.at("relaxed")->getBool();
    Chrono chrono;
    PreprocessedDatas pdatas(landscape, plan);
    OSI_Builder solver_builder = OSI_Builder();
    Variables vars(landscape, plan, pdatas);
    insert_variables(solver_builder, vars);
    if(log_level > 0) std::cout << name() << ": Start filling solver : " << solver_builder.getNbVars() << " variables" << std::endl;
    fill_solver(solver_builder, landscape, plan, B, vars, relaxed, pdatas);
    OsiSolverInterface * solver = solver_builder.buildSolver<OsiGrbSolverInterface>(OSI_Builder::MAX);
       if(log_level <= 1) solver->setHintParam(OsiDoReducePrint);
    if(log_level >= 1) {
        if(log_level >= 2) {
            name_variables(solver_builder, landscape, plan, vars, pdatas);
            OsiClpSolverInterface * solver_clp = solver_builder.buildSolver<OsiClpSolverInterface>(OSI_Builder::MAX);
            solver_clp->writeLp("pl_eca_2");
            delete solver_clp;
            std::cout << name() << ": LP printed to 'pl_eca_2.lp'" << std::endl;
        }
        std::cout << name() << ": Complete filling solver : " << solver_builder.getNbConstraints() << " constraints in " << chrono.lapTimeMs() << " ms" << std::endl;
        std::cout << name() << ": Start solving" << std::endl;
    }
    ////////////////////
    solver->branchAndBound();
    ////////////////////
    const double * var_solution = solver->getColSolution();
    if(var_solution == nullptr) {
        std::cerr << name() << ": Fail" << std::endl;
        delete solver;
        return nullptr;
    }
    Solution * solution = new Solution(landscape, plan);
    for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i) {
        const int y_i = vars.y.id(i);
        const double value = var_solution[y_i];
        solution->set(i, value);
    }
    solution->setComputeTimeMs(chrono.timeMs());
    solution->obj = solver->getObjValue();
    solution->nb_vars = solver->getNumCols();
    solution->nb_constraints = solver->getNumRows();
    if(log_level >= 1) {
        std::cout << name() << ": Complete solving : " << solution->getComputeTimeMs() << " ms" << std::endl;
        std::cout << name() << ": ECA from obj : " << std::sqrt(solution->obj) << std::endl;
    }
    delete solver;

    return solution;
}


double Solvers::PL_ECA_2::eval(const Landscape & landscape, const RestorationPlan<Landscape>& plan, const double B, const Solution & solution) const {
    const int log_level = params.at("log")->getInt();
    OSI_Builder solver_builder = OSI_Builder();
    PreprocessedDatas pdatas(landscape, plan);
    Variables vars(landscape, plan, pdatas);
    insert_variables(solver_builder, vars);
    if(log_level > 0) std::cout << name() << ": Start eval" << std::endl;
    fill_solver(solver_builder, landscape, plan, B, vars, false, pdatas);
    for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i) {
        const int y_i = vars.y.id(i);
        double y_i_value = solution[i];
        solver_builder.setBounds(y_i, y_i_value, y_i_value);
    }
    OsiSolverInterface * solver = solver_builder.buildSolver<OsiClpSolverInterface>(OSI_Builder::MAX);
    if(log_level <= 1) solver->setHintParam(OsiDoReducePrint);
    ////////////////////
    solver->initialSolve();  
    ////////////////////  
    const double * var_solution = solver->getColSolution();
    if(var_solution == nullptr) {
        std::cerr << name() << ": eval failed" << std::endl;
        return 0.0;
    }
    double obj = solver->getObjValue();
    if(log_level >= 1) std::cout << name() << ": eval : " << obj << std::endl;
    delete solver;
    return obj;
}