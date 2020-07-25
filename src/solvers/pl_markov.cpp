#include "solvers/pl_markov.hpp"

Graph_t::ArcMap<std::map<Graph_t::Arc, double>> * PL_Markov_Solver::getRhoMap(const Landscape & landscape) const {
    const Graph_t & graph = landscape.getNetwork();

    Graph_t::ArcMap<std::map<Graph_t::Arc, double>> * rhoMap = new Graph_t::ArcMap<std::map<Graph_t::Arc, double>>(graph);

    /*for(Graph_t::ArcIt a(graph); a!=lemon::INVALID; ++a) {
        Graph_t::Node v = graph.target(a);

        std::vector<Graph_t::Arc> outArcs;
        for(Graph_t::OutArcIt b(graph, v); b!=lemon::INVALID; ++b) {
            outArcs.push_back(b);
        }

        auto vector = [&] (Graph_t::Arc x) {
            Point _u = landscape.getCoords(graph.source(x));
            Point _v = landscape.getCoords(graph.target(x));
            Eigen::Vector3d vx(_v.x - _u.x, _v.y - _u.y, 0);
            return vx;
        };
        auto cross = [&] (Eigen::Vector3d vx, Eigen::Vector3d vy) {
            return vx.cross(vy)[2];
        };
        auto angle = [&] (Eigen::Vector3d vx, Eigen::Vector3d vy) {
            double internal_angle = std::acos(vx.dot(vy) / (vx.norm() * vy.norm()));
            if(cross(vx, vy) < 0)
                return 2*M_PI-internal_angle;
            return internal_angle;
        };
        auto comp = [&] (Graph_t::Arc x, Graph_t::Arc y) {
            Eigen::Vector3d vx = vector(x);
            Eigen::Vector3d vy = vector(y);
            Eigen::Vector3d v0(1, 0, 0);
            
            const double c0x = cross(v0, vx);
            const double c0y = cross(v0, vy);

            if(c0x == 0 && c0y == 0)
                return vx[0] > vy[0];

            if((c0x > 0) ^ (c0y > 0))
                return c0x > 0;

            const double cxy = cross(vx, vy);

            return cxy > 0;
        };

        std::sort(outArcs.begin(), outArcs.end(), comp);

        const int nb_arcs = outArcs.size();
        for(int i=1; i<=nb_arcs; i++) {
            Graph_t::Arc b = outArcs[i % nb_arcs];
            double surrounding_angle = angle(vector(outArcs[i-1]), vector(outArcs[(i+1) % nb_arcs]));
            double portion = surrounding_angle / (4 * M_PI);

            (*rhoMap)[a][b] = portion;

            
            //Eigen::Vector3d v0(1, 0, 0);
            //std::cout << " angle : " << angle(v0, vector(outArcs[i-1])) << " " << portion << std::endl;
        }
        //std::cout << std::endl;
    }*/

    for(Graph_t::ArcIt a(graph); a!=lemon::INVALID; ++a) {
        Graph_t::Node v = graph.target(a);
        const int nb_arcs = lemon::countOutArcs(graph, v);
        for(Graph_t::OutArcIt b(graph, v); b!=lemon::INVALID; ++b) {
            (*rhoMap)[a][b] = 1 / nb_arcs;
        }
    }

    return rhoMap;
}

Solution * PL_Markov_Solver::solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const int nb_threads = params.at("threads")->getInt();
    const bool relaxed = params.at("relaxed")->getBool();

    const Graph_t & graph = landscape.getNetwork();
    const Graph_t::NodeMap<double> & qualityMap = landscape.getQualityMap();
    const Graph_t::ArcMap<double> & difficultyMap = landscape.getDifficultyMap();
    
    const int n = lemon::countNodes(graph);
    const int m = lemon::countArcs(graph);

    Graph_t::ArcMap<std::map<Graph_t::Arc, double>> * rhoMap = getRhoMap(landscape);

    auto x_var = [&] (Graph_t::Node t, Graph_t::Arc a) {
        const int id_t = graph.id(t);
        const int id_a = graph.id(a);
        return id_t*(2*m) + id_a;
    };
    const int x_var_number = n*(2*m);
    
    auto y_var = [&] (Graph_t::Arc e) {
        assert(plan.contains(e));
        return x_var_number + plan.id(e);
    };
    const int y_var_number = plan.edges().size();


    auto P_const = [&] (Graph_t::Arc e) {
        return std::exp(- difficultyMap[e] / alpha);
    };
    auto rho_const = [&] (Graph_t::Arc a, Graph_t::Arc b) {
        const Graph_t::Node u = graph.target(a);
        if(graph.source(b) != u)
            return 0.0;
        return (*rhoMap)[a][b];
    };
    auto lambda_const = [&] (Graph_t::Arc b) {
        const Graph_t::Node u = graph.source(b); 
        return qualityMap[u] / lemon::countOutArcs(graph, u);
    };
    auto gamma_const = [&] (Graph_t::Arc b) {
        //TODO better
        (void)b;
        return 100000;
    };

    OSI_Builder * solver_builder = new OSI_Builder();

    solver_builder->addVarType(x_var_number, NULL, 0, solver_builder->infty());
    solver_builder->addVarType(y_var_number, NULL, 0, 1);
    solver_builder->init();

    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    // Columns
    for(typename Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
        for(typename Graph_t::InArcIt a(graph, t); a != lemon::INVALID; ++a) {
            if(graph.source(a) == t)
                continue;
            const int x_t_a = x_var(t, a);
            solver_builder->setObjective(x_t_a, 1);
        }
    }

    // Rows
    for(typename Graph_t::NodeIt t(graph); t != lemon::INVALID; ++t) {
        // flow constraints: --a-->(u)--b-->       
        // x_b = P(b) * (\sum rho_ab x_a + lambda_b)
        for(typename Graph_t::ArcIt b(graph); b != lemon::INVALID; ++b) {
            const int x_t_b = x_var(t,b);
            solver_builder->buffEntry(x_t_b, 1);

            const Graph_t::Node u = graph.source(b);
            for(typename Graph_t::InArcIt a(graph, u); a != lemon::INVALID; ++a) {
                const int x_t_a = x_var(t, a);
                solver_builder->buffEntry(x_t_a, -rho_const(a,b)*P_const(b));
            }
            solver_builder->pushRow(-solver_builder->infty(), P_const(b) * lambda_const(b));
        }
        // patch suppression constraint: --a-->(u)
        // x_a < gamma_e y_e
        for(Graph_t::Arc e : plan.edges()) {
            const int y_e = y_var(e);
            for(bool direction : {false, true}) {
                Graph_t::Arc a = graph.direct(e, direction);
                const int x_t_a = x_var(t,a);               
                solver_builder->buffEntry(x_t_a, 1);
                solver_builder->buffEntry(y_e, -gamma_const(a));
                
                solver_builder->pushRow(-solver_builder->infty(), 0);
            }
        }
    }
    // budget constraint
    for(Graph_t::Arc e : plan.edges()) {
            const int y_e = y_var(e);
            solver_builder->buffEntry(y_e, plan.getCost(e));
    }
    solver_builder->pushRow(0, B);
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    
    OsiClpSolverInterface * solver = solver_builder->buildSolver(OSI_Builder::MAX);

    if(!relaxed) {
        for(Graph_t::Arc e : plan.edges()) {
            const int y_e = y_var(e);
            solver->setInteger(y_e);
        }
    }

    solver->writeLp("test_markov");

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

    Solution * solution = new Solution(landscape);

    for(Graph_t::Arc e : plan.edges()) {
        const int y_e = y_var(e);
        if(var_solution[y_e] == 0)
            continue;
        solution->set(e, var_solution[y_e]);
    }

    delete solver;
    delete solver_builder;

    return solution;
}