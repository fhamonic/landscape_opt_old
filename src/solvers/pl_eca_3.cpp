#include "solvers/pl_eca_3.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

double max_flow_in(const Landscape & landscape, const RestorationPlan & plan, Graph_t::Node t) {
    typedef lemon::ReverseDigraph<const Graph_t> Reversed;
    const Graph_t & original_g = landscape.getNetwork();
    Reversed reversed_g(original_g); 
    Graph_t::ArcMap<double> probabilities(original_g);
    for(Graph_t::ArcIt b(original_g); b != lemon::INVALID; ++b)
        probabilities[b] = landscape.getProbability(b);
    for(RestorationPlan::Option * option : plan.options())
        for(Graph_t::Arc b : option->arcs())
            probabilities[b] = std::max(probabilities[b], option->getRestoredProbability(b));
    lemon::MultiplicativeSimplerDijkstra<Reversed, Graph_t::ArcMap<double>> dijkstra(reversed_g, probabilities);
    double sum = 0;
    dijkstra.init(t);
    while (!dijkstra.emptyQueue()) {
        std::pair<Graph_t::Node, double> pair = dijkstra.processNextNode();
        Graph_t::Node v = pair.first;
        const double p_tv = pair.second;
        sum += landscape.getQuality(v) * p_tv;
        for(RestorationPlan::Option * option : plan.getOptions(v))
            sum += option->getQualityGain(v) * p_tv;
    }
    return sum;
}

class PreprocessedDatas {
    public:
        std::vector<Graph_t::Node> target_nodes;
        Graph_t::NodeMap<ContractionResult> * contracted_instances;
        Graph_t::NodeMap<Graph_t::NodeMap<double>*> M_Maps_Map;

        PreprocessedDatas(const Landscape & landscape, const RestorationPlan & plan) : M_Maps_Map(landscape.getNetwork()) {
            const Graph_t & graph = landscape.getNetwork();
            // target_nodes
            for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) { 
                if(landscape.getQuality(u) == 0 && !plan.contains(u)) continue;
                target_nodes.push_back(u); 
            }
            // contracted_instances
            MyContractionAlgorithm alg2;
            contracted_instances = alg2.precompute(landscape, plan);
            // M_Maps_Map
            std::for_each(std::execution::par, target_nodes.begin(), target_nodes.end(), [&] (Graph_t::Node t) {
                const ContractionResult & cr = (*contracted_instances)[t];
                const Landscape & contracted_landscape = *cr.landscape;
                const Graph_t & contracted_graph = contracted_landscape.getNetwork();
                const RestorationPlan & contracted_plan = *cr.plan;

                M_Maps_Map[t] = new Graph_t::NodeMap<double>(contracted_graph);
                Graph_t::NodeMap<double> & M_Map = *M_Maps_Map[t];
                for(Graph_t::NodeIt v(contracted_graph); v != lemon::INVALID; ++v)
                    M_Map[v] = max_flow_in(contracted_landscape, contracted_plan, v);
            });
        }
        ~PreprocessedDatas() {
            for(Graph_t::Node v : target_nodes) delete  M_Maps_Map[v];
            delete contracted_instances;
        }
};

namespace Solvers::PL_ECA_3_Vars {
    class XVar : public OSI_Builder::VarType {
        private:
            const ContractionResult & _cr;
        public:
            XVar(const ContractionResult & cr): VarType(lemon::countArcs(cr.landscape->getNetwork())), _cr(cr) {}
            int id(Graph_t::Arc a) const { 
                assert(_cr.landscape->getNetwork().valid(a)); 
                const int id = _cr.landscape->getNetwork().id(a);
                assert(id >=0 && id < _number); return _offset + id; 
            }
    };
    class RestoredXVar : public OSI_Builder::VarType {
        private:
            const ContractionResult & _cr;
            std::map<RestorationPlan::Option *, int> offsets;
        public:
            RestoredXVar(const ContractionResult & cr): _cr(cr) {
                int cpt = 0;
                for(RestorationPlan::Option * option : cr.plan->options()) { offsets[option] = cpt; cpt += option->getNbArcs(); }
                _number = cpt;
            }
            int id(Graph_t::Arc a, RestorationPlan::Option * option) const {
                assert(_cr.landscape->getNetwork().valid(a) && offsets.contains(option)); 
                const int id = offsets.at(option) + option->id(a);
                assert(id >=0 && id < _number); return _offset + id; 
            }
    };
    class FVar : public OSI_Builder::VarType {
        public:
            FVar(): VarType(1) {}
            int id() const { return _offset; }
    };
    class RestoredFVar : public OSI_Builder::VarType {
        private:
            std::map<int, int> offsets;
        public:
            RestoredFVar(const ContractionResult & cr) : VarType(0) {
                for(RestorationPlan::Option * option : cr.plan->getOptions(cr.t)) {
                    offsets[option->getId()] = _number++;
                }
            }
            int id(const RestorationPlan::Option * option) const { 
                assert(offsets.contains(option->getId())); 
                const int id = offsets.at(option->getId());
                assert(id >=0 && id < _number); return _offset + id;
            }
    };
    class YVar : public OSI_Builder::VarType {
        public:
            YVar(const RestorationPlan & plan): VarType(plan.getNbOptions(), 0, 1, true) {}
            int id(const RestorationPlan::Option * option) const { 
                const int id = option->getId();
                assert(id >=0 && id < _number); return _offset + id;
            }
    };

    class ContractedVars {
        public:
            XVar x;
            RestoredXVar restored_x;
            FVar f;
            RestoredFVar restored_f;
            ContractedVars(const ContractionResult & cr) : x(cr), restored_x(cr), f(), restored_f(cr) {};
    };

    class Variables {
        public:
            Graph_t::NodeMap<ContractedVars*> contracted;
            YVar y;
            Variables(const Landscape & landscape, const RestorationPlan & plan, PreprocessedDatas & pdatas) :
                    contracted(landscape.getNetwork(), nullptr), y(plan) {
                for(Graph_t::Node v : pdatas.target_nodes)
                    contracted[v] = new ContractedVars((*pdatas.contracted_instances)[v]);
            }
            ContractedVars & operator[](Graph_t::Node t) const { return *contracted[t]; }
    };

    void name_variables(OSI_Builder & solver, const Landscape & landscape, const RestorationPlan & plan, PreprocessedDatas & pdatas, const Variables & vars) {
        const Graph_t & graph = landscape.getNetwork();
        auto node_str = [&graph] (Graph_t::Node v) { return std::to_string(graph.id(v)); };
        // XVar
        for(Graph_t::Node t : pdatas.target_nodes) {
            const ContractedVars & cvars = vars[t];
            const ContractionResult & cr = (*pdatas.contracted_instances)[t];
            const Landscape & contracted_landscape = *cr.landscape;
            const Graph_t & contracted_graph = contracted_landscape.getNetwork();
            const RestorationPlan & contracted_plan = *cr.plan;
            for(Graph_t::ArcIt a(contracted_graph); a != lemon::INVALID; ++a) {
                // solver->setColName(x_var->id(a), "x_t_" + node_str(t) + "_a_" + std::to_string(contracted_graph.id(a)));
                solver.setColName(cvars.x.id(a), "x_t_" + node_str(t) + "(" + std::to_string(contracted_graph.id(cr.t)) + ")_a_" + std::to_string(contracted_graph.id(a)) + "(" + std::to_string(contracted_graph.id(contracted_graph.source(a))) + ","+ std::to_string(contracted_graph.id(contracted_graph.target(a))) + ")" );
                // RestoredXVar
                for(RestorationPlan::Option * option : contracted_plan.getOptions(a))
                    solver.setColName(cvars.restored_x.id(a, option), "restored_x_t_" + node_str(t) + "_a_" + std::to_string(contracted_graph.id(a)) + "_" + std::to_string(option->getId()));
            }
        }
        // FVar
        for(Graph_t::Node t : pdatas.target_nodes) solver.setColName(vars[t].f.id(), "f_t_" + node_str(t));
        // RestoredFVar
        for(Graph_t::Node t : pdatas.target_nodes)
            for(RestorationPlan::Option * option : plan.getOptions(t))
                solver.setColName(vars[t].restored_f.id(option), "restored_f_t_" + node_str(t) + "_" + std::to_string(option->getId()));
        // YVar
        for(RestorationPlan::Option * option : plan.options()) solver.setColName(vars.y.id(option), "y_" + std::to_string(option->getId()));
    }
}

using namespace Solvers::PL_ECA_3_Vars;

void insert_variables(OSI_Builder & solver_builder, Variables & vars, PreprocessedDatas & pdatas) {
    for(Graph_t::Node t : pdatas.target_nodes) {
        solver_builder.addVarType(&vars[t].x);
        solver_builder.addVarType(&vars[t].restored_x);
        solver_builder.addVarType(&vars[t].f);
        solver_builder.addVarType(&vars[t].restored_f);
    }
    solver_builder.addVarType(&vars.y);
    solver_builder.init();
}

void fill_solver(OSI_Builder & solver_builder, const Landscape & landscape, const RestorationPlan & plan, const double B, 
        Variables & vars, bool relaxed, PreprocessedDatas & pdatas) {

    auto M_x_const = [&] (Graph_t::Node t, Graph_t::Arc a) {
        const ContractionResult & cr = (*pdatas.contracted_instances)[t];
        const Graph_t & contracted_graph = cr.landscape->getNetwork();
        return (*pdatas.M_Maps_Map[t])[contracted_graph.source(a)];  
    };
    auto M_f_const = [&] (Graph_t::Node t) {
        const ContractionResult & cr = (*pdatas.contracted_instances)[t];
        return (*pdatas.M_Maps_Map[t])[cr.t];
    };
    ////////////////////////////////////////////////////////////////////////
    // Columns : Objective
    ////////////////////
    for(Graph_t::Node t : pdatas.target_nodes) {
        // sum w(t) * f_t
        const int f_t = vars[t].f.id();
        solver_builder.setObjective(f_t, landscape.getQuality(t));
        for(RestorationPlan::Option * option : plan.getOptions(t)) {
            const int restored_f_t = vars[t].restored_f.id(option);
            solver_builder.setObjective(restored_f_t, option->getQualityGain(t));
        }
    }
    ////////////////////////////////////////////////////////////////////////
    // Rows : Constraints
    ////////////////////
    for(Graph_t::Node t : pdatas.target_nodes) {
        const ContractedVars & cvars = vars[t];
        const int f_t = cvars.f.id();
        const ContractionResult & cr = (*pdatas.contracted_instances)[t];
        const Landscape & contracted_landscape = *cr.landscape;
        const Graph_t & contracted_graph = contracted_landscape.getNetwork();
        const RestorationPlan & contracted_plan = *cr.plan;
        // out_flow(u) - in_flow(u) <= w(u)
        for(Graph_t::NodeIt u(contracted_graph); u != lemon::INVALID; ++u) {
            // out flow
            for(Graph_t::OutArcIt b(contracted_graph, u); b != lemon::INVALID; ++b) {
                const int x_tb = cvars.x.id(b);
                solver_builder.buffEntry(x_tb, 1);
                for(RestorationPlan::Option * option : contracted_plan.getOptions(b)) {
                    const int restored_x_t_b = cvars.restored_x.id(b, option);
                    solver_builder.buffEntry(restored_x_t_b, 1);
                }
            }
            // in flow
            for(Graph_t::InArcIt a(contracted_graph, u); a != lemon::INVALID; ++a) {
                const int x_ta = cvars.x.id(a);
                solver_builder.buffEntry(x_ta, -contracted_landscape.getProbability(a));
                for(RestorationPlan::Option * option : contracted_plan.getOptions(a)) {
                    const int degraded_x_t_a = cvars.restored_x.id(a, option);
                    solver_builder.buffEntry(degraded_x_t_a, -option->getRestoredProbability(a));
                }
            }
            // optional injected flow
            for(RestorationPlan::Option * option : contracted_plan.getOptions(u)) {
                const int y_u = vars.y.id(option);
                solver_builder.buffEntry(y_u, -option->getQualityGain(u));
            }
            // optimisation variable
            if(u == cr.t) solver_builder.buffEntry(f_t, 1);
            // injected flow
            solver_builder.pushRow(-OSI_Builder::INFTY, contracted_landscape.getQuality(u));
        }
        // restored_x_a < y_i * M
        for(RestorationPlan::Option * option : contracted_plan.options()) {
            const int y_i = vars.y.id(option);
            for(Graph_t::Arc a : option->arcs()) {
                const int x_ta = cvars.restored_x.id(a, option);
                solver_builder.buffEntry(y_i, M_x_const(t, a));
                solver_builder.buffEntry(x_ta, -1);
                solver_builder.pushRow(0, OSI_Builder::INFTY);
            }
        }
    }
    // restored_f_t <= f_t
    // restored_f_t <= y_i * M
    for(RestorationPlan::Option * option : plan.options()) {
        const int y_i = vars.y.id(option);
        for(Graph_t::Node t : option->nodes()) {
            const ContractedVars & cvars = vars[t];
            const int f_t = cvars.f.id();
            const int restored_f_t = cvars.restored_f.id(option);
            solver_builder.buffEntry(f_t, 1);
            solver_builder.buffEntry(restored_f_t, -1);
            solver_builder.pushRow(0, OSI_Builder::INFTY);

            solver_builder.buffEntry(y_i, M_f_const(t));
            solver_builder.buffEntry(restored_f_t, -1);
            solver_builder.pushRow(0, OSI_Builder::INFTY);
        }
    }
    ////////////////////
    // sum y_i < B
    for(RestorationPlan::Option * option : plan.options()) {
        const int y_i = vars.y.id(option);
        solver_builder.buffEntry(y_i, option->getCost());
    }
    solver_builder.pushRow(0, B);
    ////////////////////
    // integer constraints
    if(!relaxed) {
        for(RestorationPlan::Option * option : plan.options()) {
            const int y_i = vars.y.id(option);
            solver_builder.setInteger(y_i);
        }
    }
}

Solution * Solvers::PL_ECA_3::solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const bool relaxed = params.at("relaxed")->getBool();
    Chrono chrono;
    if(log_level > 0) std::cout << name() << ": Start preprocessing" << std::endl;
    PreprocessedDatas preprocessed_datas(landscape, plan);
    OSI_Builder solver_builder = OSI_Builder();
    Variables vars(landscape, plan, preprocessed_datas);
    insert_variables(solver_builder, vars, preprocessed_datas);
    if(log_level > 0) {
        std::cout << name() << ": Complete preprocessing : " << chrono.lapTimeMs() << " ms" << std::endl;
        std::cout << name() << ": Start filling solver : " << solver_builder.getNbVars() << " variables" << std::endl;
    }
    fill_solver(solver_builder, landscape, plan, B, vars, relaxed, preprocessed_datas);
    OsiSolverInterface * solver = solver_builder.buildSolver<OsiGrbSolverInterface>(OSI_Builder::MAX);
    if(log_level <= 1) solver->setHintParam(OsiDoReducePrint);
    if(log_level >= 1) {
        if(log_level >= 2) {
            name_variables(solver_builder, landscape, plan, preprocessed_datas, vars);
            OsiClpSolverInterface * solver_clp = solver_builder.buildSolver<OsiClpSolverInterface>(OSI_Builder::MAX);
            solver_clp->writeLp("pl_eca_3");
            delete solver_clp;
            std::cout << name() << ": LP printed to 'pl_eca_3.lp'" << std::endl;
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
    for(RestorationPlan::Option * option : plan.options()) {
        const int y_i = vars.y.id(option);
        double value = var_solution[y_i];
        solution->set(option, value);
    }
    solution->setComputeTimeMs(chrono.timeMs());
    solution->obj = solver->getObjValue();
    solution->nb_vars = solver_builder.getNbVars();
    solution->nb_constraints = solver_builder.getNbConstraints();
    if(log_level >= 1) {
        std::cout << name() << ": Complete solving : " << solution->getComputeTimeMs() << " ms" << std::endl;
        std::cout << name() << ": ECA from obj : " << std::sqrt(solution->obj) << std::endl;
    }    
    delete solver;
    return solution;
}

double Solvers::PL_ECA_3::eval(const Landscape & landscape, const RestorationPlan & plan, const double B, const Solution & solution) const {
    const int log_level = params.at("log")->getInt();
    const bool relaxed = params.at("relaxed")->getBool();
    Chrono chrono;
    PreprocessedDatas preprocessed_datas(landscape, plan);
    OSI_Builder solver_builder = OSI_Builder();
    Variables vars(landscape, plan, preprocessed_datas);
    insert_variables(solver_builder, vars, preprocessed_datas);
    fill_solver(solver_builder, landscape, plan, B, vars, relaxed, preprocessed_datas);
    for(auto option_pair : solution.getOptionCoefs()) {
        const RestorationPlan::Option * option = option_pair.first;
        const int y_i = vars.y.id(option);
        double y_i_value = option_pair.second;
        solver_builder.setBounds(y_i, y_i_value, y_i_value);
    }
    OsiSolverInterface * solver = solver_builder.buildSolver<OsiGrbSolverInterface>(OSI_Builder::MAX);
    ////////////////////
    solver->initialSolve();  
    //////////////////// 
    const double * var_solution = solver->getColSolution();
    if(var_solution == nullptr) {
        std::cerr << name() << ": Fail" << std::endl;
        delete solver;
        return 0.0;
    }
    double obj = solver->getObjValue();
    if(log_level >= 1) std::cout << name() << ": eval : " << obj << std::endl;
    delete solver;
    return obj;
}