#include "solvers/pl_eca_3.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

namespace Solvers::PL_ECA_3_Vars {
    class XVar : public OSI_Builder::VarType {
        private:
            const ContractionResult & _cr;
        public:
            XVar(ContractionResult & cr): VarType(lemon::countArcs(cr.landscape->getNetwork())), _cr(cr) {}
            int id(Graph_t::Arc a) { 
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
            RestoredXVar(ContractionResult & cr): _cr(cr) {
                int cpt = 0;
                for(RestorationPlan::Option * option : cr.plan->options()) { offsets[option] = cpt; cpt += option->getNbArcs(); }
                _number = cpt;
            }
            int id(Graph_t::Arc a, RestorationPlan::Option * option) {
                assert(_cr.landscape->getNetwork().valid(a) && offsets.contains(option)); 
                const int id = offsets.at(option) + option->id(a);
                assert(id >=0 && id < _number); return _offset + id; 
            }
    };
    class FVar : public OSI_Builder::VarType {
        private:
            const Landscape & _landscape;
        public:
            FVar(const Landscape & landscape): VarType(lemon::countNodes(landscape.getNetwork())), _landscape(landscape) {}
            int id(Graph_t::Node u) {
                assert(_landscape.getNetwork().valid(u)); 
                const int id = _landscape.getNetwork().id(u);
                assert(id >=0 && id < _number); return _offset + id;
            }
    };
    class RestoredFVar : public OSI_Builder::VarType {
        private:
            const RestorationPlan & _plan;
            std::map<const RestorationPlan::Option *, int> offsets;
        public:
            RestoredFVar(const RestorationPlan & plan): _plan(plan) {
                int cpt = 0;
                for(RestorationPlan::Option * option : plan.options()) { offsets[option] = cpt; cpt += option->getNbNodes(); }
                _number = cpt;
            }
            int id(Graph_t::Node a, const RestorationPlan::Option * option) { 
                assert(offsets.contains(option)); 
                const int id = offsets.at(option) + option->id(a);
                assert(id >=0 && id < _number); return _offset + id;
            }
    };
    class YVar : public OSI_Builder::VarType {
        public:
            YVar(const RestorationPlan & plan): VarType(plan.getNbOptions(), 0, 1, true) {}
            int id(const RestorationPlan::Option * option) { 
                const int id = option->getId();
                assert(id >=0 && id < _number); return _offset + id;
            }
    };

    typedef struct {
        XVar * x_var = nullptr;
        RestoredXVar * restored_x_var = nullptr;
    } Vars;

    void name_variables(OsiSolverInterface * solver, const Landscape & landscape, const RestorationPlan & plan, const std::vector<Graph_t::Node> target_nodes, Graph_t::NodeMap<ContractionResult> * contracted_instances, Graph_t::NodeMap<Vars> & varsMap, FVar & f_var, RestoredFVar & restored_f_var, YVar & y_var) {
        const Graph_t & graph = landscape.getNetwork();
        
        auto node_str = [&graph] (Graph_t::Node v) { return std::to_string(graph.id(v)); };
        
        // XVar
        for(Graph_t::Node t : target_nodes) {
            XVar * x_var = varsMap[t].x_var;
            RestoredXVar * restored_x_var = varsMap[t].restored_x_var;

            ContractionResult & cr = (*contracted_instances)[t];
            const Landscape * contracted_landscape = cr.landscape;
            const Graph_t & contracted_graph = contracted_landscape->getNetwork();
            const RestorationPlan * contracted_plan = cr.plan;

            for(Graph_t::ArcIt a(contracted_graph); a != lemon::INVALID; ++a) {
                // solver->setColName(x_var->id(a), "x_t_" + node_str(t) + "_a_" + std::to_string(contracted_graph.id(a)));
                solver->setColName(x_var->id(a), "x_t_" + node_str(t) + "(" + std::to_string(contracted_graph.id(cr.t)) + ")_a_" + std::to_string(contracted_graph.id(a)) + "(" + std::to_string(contracted_graph.id(contracted_graph.source(a))) + ","+ std::to_string(contracted_graph.id(contracted_graph.target(a))) + ")" );
                // RestoredXVar
                for(RestorationPlan::Option * option : contracted_plan->getOptions(a))
                    solver->setColName(restored_x_var->id(a, option), "restored_x_t_" + node_str(t) + "_a_" + std::to_string(contracted_graph.id(a)) + "_" + std::to_string(option->getId()));
            }
        }
        // FVar
        for(Graph_t::Node t : target_nodes) solver->setColName(f_var.id(t), "f_t_" +  node_str(t));
        // RestoredFVar
        for(Graph_t::Node t : target_nodes)
            for(RestorationPlan::Option * option : plan.getOptions(t))
                solver->setColName(restored_f_var.id(t, option), "restored_f_t_" + node_str(t) + "_" + std::to_string(option->getId()));
        // YVar
        for(RestorationPlan::Option * option : plan.options()) solver->setColName(y_var.id(option), "y_" + std::to_string(option->getId()));
    }
}

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


using namespace Solvers::PL_ECA_3_Vars;

Solution * Solvers::PL_ECA_3::solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const bool relaxed = params.at("relaxed")->getBool();
    // const int nb_threads = params.at("threads")->getInt();
    // const int timeout = params.at("timeout")->getInt();
    // const bool fortest = params.at("fortest")->getBool();

    std::chrono::time_point<std::chrono::high_resolution_clock> last_time, current_time;
    last_time = std::chrono::high_resolution_clock::now();
    
    Solution * solution = new Solution(landscape, plan);

    if(log_level > 0) {
        std::cout << name() << ": Start preprocessing" << std::endl;
    }

    const Graph_t & graph = landscape.getNetwork();
    std::vector<Graph_t::Node> target_nodes;
    for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) {
        // if(landscape.getQuality(u) == 0.0 && !plan.contains(u))
        //     continue;
        target_nodes.push_back(u);
    }

    // TODO : change precompute to process only target_nodes
    MyContractionAlgorithm alg2;
    Graph_t::NodeMap<ContractionResult> * contracted_instances = alg2.precompute(landscape, plan);

    Graph_t::NodeMap<Vars> varsMap(graph);
    for(Graph_t::Node v : target_nodes) {
        ContractionResult & cr = (*contracted_instances)[v];
        varsMap[v].x_var = new XVar(cr);
        varsMap[v].restored_x_var = new RestoredXVar(cr);
    }    
    FVar f_var(landscape);
    RestoredFVar restored_f_var(plan);
    YVar y_var(plan);


    Graph_t::NodeMap<Graph_t::NodeMap<double>*> M_Maps_Map(graph);

    std::for_each(std::execution::par, target_nodes.begin(), target_nodes.end(), [&] (Graph_t::Node t) {
        ContractionResult & cr = (*contracted_instances)[t];
        const Landscape * contracted_landscape = cr.landscape;
        const Graph_t & contracted_graph = contracted_landscape->getNetwork();
        const RestorationPlan * contracted_plan = cr.plan;

        M_Maps_Map[t] = new Graph_t::NodeMap<double>(contracted_graph);
        Graph_t::NodeMap<double> & M_Map = *M_Maps_Map[t];

        for(Graph_t::NodeIt v(contracted_graph); v != lemon::INVALID; ++v) {
            M_Map[v] = max_flow_in(*contracted_landscape, *contracted_plan, v);      
        }
    });


    auto M_x_const = [&] (Graph_t::Node t, Graph_t::Arc a) {
        ContractionResult & cr = (*contracted_instances)[t];
        const Graph_t & contracted_graph = cr.landscape->getNetwork();
        return (*M_Maps_Map[t])[contracted_graph.source(a)];  
    };
    auto M_f_const = [&] (Graph_t::Node t) {
        ContractionResult & cr = (*contracted_instances)[t];
        return (*M_Maps_Map[t])[cr.t]; 
    };
    
    

    OSI_Builder solver_builder = OSI_Builder();

    for(Graph_t::Node v : target_nodes) {
        Vars vars = varsMap[v];
        solver_builder.addVarType(vars.x_var);
        solver_builder.addVarType(vars.restored_x_var);
    }
    solver_builder.addVarType(&f_var);
    solver_builder.addVarType(&restored_f_var);
    solver_builder.addVarType(&y_var);

    solver_builder.init();
    
    if(log_level > 0) {
        current_time = std::chrono::high_resolution_clock::now();
        int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-last_time).count();
        solution->getComputeTimeMsRef() += time_ms;
        std::cout << name() << ": Complete preprocessing : " << time_ms << " ms" << std::endl;
        std::cout << name() << ": Start filling solver : " << solver_builder.getNbVars() << " variables" << std::endl;
        last_time = current_time;
    }

    ////////////////////////////////////////////////////////////////////////
    // Columns : Objective
    ////////////////////
    for(Graph_t::Node t : target_nodes) {
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
    for(Graph_t::Node t : target_nodes) {
        const int f_t = f_var.id(t);
        ContractionResult & cr = (*contracted_instances)[t];
        const Landscape * contracted_landscape = cr.landscape;
        const Graph_t & contracted_graph = contracted_landscape->getNetwork();
        const RestorationPlan * contracted_plan = cr.plan;
        const Vars vars = varsMap[t];
        // out_flow(u) - in_flow(u) <= w(u)
        for(Graph_t::NodeIt u(contracted_graph); u != lemon::INVALID; ++u) {
            // out flow
            for(Graph_t::OutArcIt b(contracted_graph, u); b != lemon::INVALID; ++b) {
                const int x_tb = vars.x_var->id(b);
                solver_builder.buffEntry(x_tb, 1);
                for(RestorationPlan::Option * option : contracted_plan->getOptions(b)) {
                    const int restored_x_t_b = vars.restored_x_var->id(b, option);
                    solver_builder.buffEntry(restored_x_t_b, 1);
                }
            }
            // in flow
            for(Graph_t::InArcIt a(contracted_graph, u); a != lemon::INVALID; ++a) {
                const int x_ta = vars.x_var->id(a);
                solver_builder.buffEntry(x_ta, -contracted_landscape->getProbability(a));
                for(RestorationPlan::Option * option : contracted_plan->getOptions(a)) {
                    const int degraded_x_t_a = vars.restored_x_var->id(a, option);
                    solver_builder.buffEntry(degraded_x_t_a, -option->getRestoredProbability(a));
                }
            }
            // optional injected flow
            for(RestorationPlan::Option * option : contracted_plan->getOptions(u)) {
                const int y_u = y_var.id(option);
                solver_builder.buffEntry(y_u, -option->getQualityGain(u));
            }
            // optimisation variable
            if(u == cr.t)
                solver_builder.buffEntry(f_t, 1);
            // injected flow
            solver_builder.pushRow(0.0, contracted_landscape->getQuality(u));
        }

        // restored_x_a < y_i * M
        for(RestorationPlan::Option * option : contracted_plan->options()) {
            const int y_i = y_var.id(option);
            for(Graph_t::Arc a : option->arcs()) {
                const int x_ta = vars.restored_x_var->id(a, option);
                solver_builder.buffEntry(y_i, M_x_const(t, a));
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

            solver_builder.buffEntry(y_i, M_f_const(t));
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
        solution->getComputeTimeMsRef() += time_ms;
        std::cout << name() << ": Complete filling solver : " << solver_builder.getNbConstraints() << " constraints in " << time_ms << " ms" << std::endl;
        std::cout << name() << ": Start solving" << std::endl;
    }
    last_time = current_time;

    OsiSolverInterface * solver = solver_builder.buildSolver<OsiGrbSolverInterface>(OSI_Builder::MAX);

    if(!relaxed) {
        for(RestorationPlan::Option * option : plan.options()) {
            const int y_i = y_var.id(option);
            solver->setInteger(y_i);
        }
    }
 
    if(log_level == 0)
        solver->setHintParam(OsiDoReducePrint);

    if(log_level > 1) {
        name_variables(solver, landscape, plan, target_nodes, contracted_instances, varsMap, f_var, restored_f_var, y_var);
        solver->writeLp("pl_eca_3");
        solver->writeMps("pl_eca_3");
    }
 

    ////////////////////////////////////////////////////////////////////////
    // Compute
    ////////////////////
    solver->branchAndBound();

    const double * var_solution = solver->getColSolution();
    if(var_solution == nullptr) {
        std::cerr << name() << ": Fail" << std::endl;
        delete solver;
        return nullptr;
    }

    // Fill Solution
    for(RestorationPlan::Option * option : plan.options()) {
        const int y_i = y_var.id(option);
        double value = var_solution[y_i];
        // remove torelance from solution
        value = std::max(value, 0.0);
        value = std::min(value, 1.0);

        solution->set(option, value);
    }


    current_time = std::chrono::high_resolution_clock::now();
    int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-last_time).count();
    solution->getComputeTimeMsRef() += time_ms;
    last_time = current_time;

    if(log_level >= 1) {
        std::cout << name() << ": Complete solving : " << time_ms << " ms" << std::endl;
        std::cout << name() << ": ECA from obj : " << std::sqrt(solver->getObjValue()) << std::endl;
    }
    
    solution->obj = std::sqrt(solver->getObjValue());
    solution->nb_vars = solver_builder.getNbVars();
    solution->nb_constraints = solver_builder.getNbConstraints();

    delete solver;






    // CbcModel model(*solver);

    // model.setLogLevel(log_level >= 2 ? 1 : 0);

    // if(model.haveMultiThreadSupport())  {
    //     model.setNumberThreads(nb_threads);
    //     if(log_level >= 1 && nb_threads > 1)
    //         std::cout << name() << ": Enabled multithread : " << model.getNumberThreads() << std::endl;
    // }
    // else 
    //     if(log_level >= 1 && nb_threads > 1)
    //         std::cerr << name() << ": multithread is disabled : to enable it build Cbc with --enable-cbc-parallel" << std::endl;
     
    // ////////////////////////////////////////////////////////////////////////
    // // Compute
    // ////////////////////
    // model.setMaximumSeconds(timeout);
    // model.branchAndBound();

    // //model.addHeuristic(new CbcHeuristicGreedyCover());


    
    // const double * var_solution = model.bestSolution();
    // if(var_solution == nullptr) {
    //     std::cerr << name() << ": Fail" << std::endl;
    //     delete solver;
    //     return nullptr;
    // }

    // // Fill Solution
    // for(RestorationPlan::Option * option : plan.options()) {
    //     const int y_i = y_var.id(option);
    //     double value = var_solution[y_i];
    //     // remove torelance from solution
    //     value = std::max(value, 0.0);
    //     value = std::min(value, 1.0);

    //     solution->set(option, value);
    // }


    // current_time = std::chrono::high_resolution_clock::now();
    // int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-last_time).count();
    // solution->getComputeTimeMsRef() += time_ms;
    // last_time = current_time;

    // if(log_level >= 1) {
    //     std::cout << name() << ": Complete solving : " << time_ms << " ms" << std::endl;
    //     std::cout << name() << ": ECA from obj : " << std::sqrt(model.getObjValue()) << std::endl;
    // }
    
    // solution->obj = std::sqrt(model.getObjValue());
    // solution->nb_vars = solver_builder.getNbVars();
    // solution->nb_constraints = solver_builder.getNbConstraints();

    // delete solver;

    return solution;
}