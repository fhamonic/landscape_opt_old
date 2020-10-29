#include "solvers/pl_eca.hpp"

Solution * PL_ECA_Solver::solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const int nb_threads = params.at("threads")->getInt();
    const int g = params.at("pieces")->getInt();
    const double st_thresold = params.at("thresold")->getDouble();
    const bool relaxed = params.at("relaxed")->getBool();
    const int timeout = params.at("timeout")->getInt();
    std::chrono::time_point<std::chrono::high_resolution_clock> last_time, current_time;

    const Graph_t & graph = landscape.getNetwork();
    const Graph_t::NodeMap<double> & qualityMap = landscape.getQualityMap();
    const Graph_t::ArcMap<double> & worst_difficultyMap = landscape.getDifficultyMap();
    Graph_t::ArcMap<double> best_difficultyMap(graph);
 
    const int nb_options = plan.options().size();

    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
        best_difficultyMap[a] = worst_difficultyMap[a];

        assert(plan.getOptions(a).size() <= 1);

        for(RestorationPlan::Option * option : plan.getOptions(a)) {
            best_difficultyMap[a] = std::min(best_difficultyMap[a], worst_difficultyMap[a] - option->getLengthGain(a));
        }
    }

    Graph_t::NodeMap<int> unthreatened_nodes_idMap(graph, -1);

    int cpt = 0;
    for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) {
        if(plan.contains(u))
            continue;
        unthreatened_nodes_idMap[u] = cpt;
        cpt++;
    }

    const int n = lemon::countNodes(graph);
    const int m = lemon::countArcs(graph);

    // optimization variables
    auto l_var = [&] (Graph_t::Node s, Graph_t::Node t) {
        return OSI_Builder::compose_pair(graph.id(s), graph.id(t));
    };
    const int l_var_number = OSI_Builder::nb_pairs(n);

    const int I_offset = l_var_number;
    auto I_var = [&] (Graph_t::Node s, Graph_t::Node t, Graph_t::Arc a, int status) {
        return I_offset + OSI_Builder::compose_pair(graph.id(s),graph.id(t)) * (2*m) * 2 + graph.id(a) * 2 + status;
    };
    const int I_var_number = OSI_Builder::nb_pairs(n)*(2*m)*2;

    const int x_offset = I_offset + I_var_number;
    auto x_var = [&] (int id_option) {
        assert(id_option >= 0 && id_option < nb_options);
        return x_offset + id_option;
    };
    const int x_var_number = nb_options;

    const int H_offset = x_offset + x_var_number;
    std::function<int (Graph_t::Node,Graph_t::Node,int)> H_var = [&] (Graph_t::Node s, Graph_t::Node t, int i) {
        assert(i >= 0 && i < g);
        return H_offset + OSI_Builder::compose_pair(graph.id(s),graph.id(t)) * g + i;
    };
    const int H_var_number = OSI_Builder::nb_pairs(n)*g;

    const Eigen::MatrixXd * best_distances = distance_matrix(graph, best_difficultyMap);
    const Eigen::MatrixXd * worst_distances = distance_matrix(graph, worst_difficultyMap);


    // piecewise optimization constants
    auto P_func = [&] (double d) {
        return std::exp(-d/alpha);
    };
    auto inv_P_func = [&] (double v) {
        return -alpha * std::log(v);
    };

    auto o_const = [&] (Graph_t::Node s, Graph_t::Node t, int i) {
        const int id_s = graph.id(s), id_t = graph.id(t);

        const double P_max = P_func((*best_distances)(id_s, id_t));
        const double P_min = P_func((*worst_distances)(id_s, id_t));

        return P_max - (P_max-P_min)/(double)(g-1) * i;
    };
    auto m_const = [&] (Graph_t::Node s, Graph_t::Node t, int i) {
        return inv_P_func(o_const(s,t,i));
    };

    OSI_Builder solver_builder = OSI_Builder();

    solver_builder.addVarType(l_var_number, nullptr, 0, solver_builder.infty());
    solver_builder.addVarType(I_var_number, nullptr, 0, 1);
    solver_builder.addVarType(x_var_number, nullptr, 0, 1);
    solver_builder.addVarType(H_var_number, nullptr, 0, 1);
    solver_builder.init();

    if(log_level > 0)
        std::cout << name() << ": Start filling solver : " << solver_builder.getNbVars() << " variables" << std::endl;
    last_time = std::chrono::high_resolution_clock::now();


    if(solver_builder.getNbVars() > 1000000) {
        return nullptr;
    }


    ////////////////////////////////////////////////////////////////////////
    // Columns : Objective
    ////////////////////
    // sum H_sti * o_sti
    for(Graph_t::NodeIt s(graph); s != lemon::INVALID; ++s) {
        for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
            const int id_s = graph.id(s);
            const int id_t = graph.id(t);
            if(id_s >= id_t)
                continue;
            if(qualityMap[s] * qualityMap[t] * P_func((*best_distances)(id_s,id_t)) <= st_thresold)
                continue;

            for(int i=0; i<g; i++)
                solver_builder.setObjective(H_var(s,t,i), o_const(s,t,i));
        }
    }
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    // Rows : Constraints
    ////////////////////
    // l_st >= sum I_sta * l(a)
    for(Graph_t::NodeIt s(graph); s != lemon::INVALID; ++s) {
        for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
            if(graph.id(s) >= graph.id(t))
                continue;
            const int l_st = l_var(s,t);
            solver_builder.buffEntry(l_st, 1);
            for(typename Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
                const int I_sta_r = I_var(s,t,a,RESTORED);
                solver_builder.buffEntry(I_sta_r, -best_difficultyMap[a]);
                const int I_sta_d = I_var(s,t,a,DEFAULT);
                solver_builder.buffEntry(I_sta_d, -worst_difficultyMap[a]);
            }
            solver_builder.pushRow(0, solver_builder.infty());
        }
    }
    ////////////////////
    // I_sta flow constraints
    // in_flow - out_flow >= bound  
    for(Graph_t::NodeIt s(graph); s != lemon::INVALID; ++s) {
        for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
            if(graph.id(s) >= graph.id(t))
                continue;
            for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
                for(typename Graph_t::InArcIt a(graph, v); a != lemon::INVALID; ++a) {
                    const int I_sta_r = I_var(s,t,a,RESTORED);
                    solver_builder.buffEntry(I_sta_r, 1);
                    const int I_sta_d = I_var(s,t,a,DEFAULT);
                    solver_builder.buffEntry(I_sta_d, 1);
                }
                for(typename Graph_t::OutArcIt a(graph, v); a != lemon::INVALID; ++a) {
                    const int I_sta_r = I_var(s,t,a,RESTORED);
                    solver_builder.buffEntry(I_sta_r, -1);
                    const int I_sta_d = I_var(s,t,a,DEFAULT);
                    solver_builder.buffEntry(I_sta_d, -1);
                }
                    
                const int bound = (v == s) ? -1 : (v == t) ? 1 : 0;
                solver_builder.pushRow(bound, solver_builder.infty());
            }
        }
    }
    ////////////////////
    // I_sta_p <= x_e
    for(Graph_t::NodeIt s(graph); s != lemon::INVALID; ++s) {
        for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
            if(graph.id(s) >= graph.id(t))
                continue;

            for(int i=0; i<nb_options; i++) {
                RestorationPlan::Option * option = plan.options()[i];
                const double x_i = x_var(i);
                for(Graph_t::Arc a : option->arcs()) {
                    const int I_sta_r = I_var(s,t,a,RESTORED);
                    solver_builder.buffEntry(I_sta_r, 1);
                    solver_builder.buffEntry(x_i, -1);
                    solver_builder.pushRow(-solver_builder.infty(), 0);
                }
            }
        }
    }
    ////////////////////
    // H_sti  ->  l_st <= m_i
    // sum H_sti <= 1
    for(Graph_t::NodeIt s(graph); s != lemon::INVALID; ++s) {
        for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
            if(graph.id(s) >= graph.id(t))
                continue;
            const int l_st = l_var(s,t);

            for(int i=0; i<g; i++) {
                const int H_sti = H_var(s,t,i);
                solver_builder.buffEntry(l_st, 1);
                solver_builder.buffEntry(H_sti, (m_const(s,t,g-1)-m_const(s,t,i)));
                solver_builder.pushRow(0, m_const(s,t,g-1));
            }

            for(int i=0; i<g; i++) {
                const int H_sti = H_var(s,t,i);
                solver_builder.buffEntry(H_sti, 1);
            }
            solver_builder.pushRow(0, 1);
        }
    }
    ////////////////////
    // sum x_e < B
    for(int i=0; i<nb_options; i++) {
        RestorationPlan::Option * option = plan.options()[i];
        const double x_i = x_var(i);
        solver_builder.buffEntry(x_i, option->getCost());
    }
    solver_builder.pushRow(0, B);
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    
    current_time = std::chrono::high_resolution_clock::now();
    if(log_level >= 1) {
        int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-last_time).count();
        std::cout << "Complete filling solver : " << solver_builder.getNbConstraints() << " constraints in " << time_ms << " ms" << std::endl;
        std::cout << "Start solving" << std::endl;
    }
    last_time = current_time;

    OsiClpSolverInterface * solver = solver_builder.buildSolver(OSI_Builder::MAX);

    for(Graph_t::NodeIt s(graph); s != lemon::INVALID; ++s) {
        for(Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
            if(graph.id(s) >= graph.id(t))
                continue;
            for(int i=0; i<g; i++) {
                const int H_sti = H_var(s,t,i);
                solver->setInteger(H_sti);
            }
        }
    }

    if(!relaxed) {
        for(int i=0; i<nb_options; i++) {
            const double x_i = x_var(i);
            solver->setInteger(x_i);
        }
    }

    if(log_level >= 1)
        solver->writeLp("test_eca");

    CbcModel model(*solver);

    model.setLogLevel(log_level >= 2 ? 1 : 0);
    if(nb_threads > 1) {
        if(model.haveMultiThreadSupport()) 
            model.setNumberThreads(nb_threads);
        else
            std::cout << "Warning: Cbc multithreaded is disabled" << std::endl;
    }
 
    model.setMaximumSeconds(timeout);
    model.branchAndBound();
    
    const double * var_solution = model.bestSolution();
    if(var_solution == nullptr) {
        std::cerr << "Fail " << model.getCurrentSeconds() << std::endl;

        

        return nullptr;
    }

    Solution * solution = new Solution(landscape, plan);

    for(int i=0; i<nb_options; i++) {
        RestorationPlan::Option * option = plan.options()[i];
        const int y_i = x_var(i);
        const double value = var_solution[y_i];
        
        solution->set(option, value);
    }


    current_time = std::chrono::high_resolution_clock::now();
    int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-last_time).count();
    solution->setComputeTimeMs(time_ms);
    last_time = current_time;
    if(log_level >= 1)
        std::cout << "Complete solving : " << time_ms << " ms" << std::endl;



    const double obj = model.getObjValue();
    double eca = 2*obj;
    for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) {
        eca += qualityMap[u]*qualityMap[u];
    }
    std::cout << "ECA from obj : " << std::sqrt(eca) << std::endl;



    solution->obj = std::sqrt(eca);
    solution->nb_vars = solver_builder.getNbVars();
    solution->nb_constraints = solver_builder.getNbConstraints();


    delete solver;

    return solution;
}