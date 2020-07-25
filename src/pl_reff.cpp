#include <iostream>
#include <chrono>

#include "lemon/list_graph.h"
#include "lemon/list_graph.h"

#include "osi_builder.hpp"

#include "coin/CbcModel.hpp"

#include "Eigen/Dense"


typedef lemon::ListDigraph Graph_t;

template <typename GR, typename DM>
void getReff(const GR & graph, const DM & resistance, typename GR::Node s) {
    const int n = lemon::countNodes(graph);

    Eigen::MatrixXd L(n, n);
    L.setZero();

    for (typename GR::ArcIt e(graph); e != lemon::INVALID; ++e) {
        const int u = graph.id(graph.source(e));
        const int v = graph.id(graph.target(e));

        L(u, v) = L(v, u) = -1/resistance[e];
        L(u, u) += 1/resistance[e];
        L(v, v) += 1/resistance[e];
    }

    // laplacian pseudoinverse of each connected component
    Eigen::MatrixXd L_plus = L.completeOrthogonalDecomposition().pseudoInverse();

    for(typename GR::NodeIt v(graph); v != lemon::INVALID; ++v) {
        const int id_u = graph.id(s);
        const int id_v = graph.id(v);

        const double Ruv = L_plus(id_u, id_u) + L_plus(id_v, id_v) - 2 * L_plus(id_u, id_v);
        
        std::cout << "IL R( "<< id_u << " , " << id_v << " ) : " << Ruv << std::endl;
    }
}

namespace Solvers::PL_Reff {
    class XVar : public OSI_Builder::VarType {
        private:
            const Graph_t & _graph;
        public:
            XVar(const Graph_t & graph): VarType(lemon::countArcs(graph), -OSI_Builder::INFTY, OSI_Builder::INFTY, false), _graph(graph) {}
            int id(Graph_t::Arc a) { const int id = _graph.id(a);
                assert(id >=0 && id < _number); return _offset + id; }
    };
    class YVar : public OSI_Builder::VarType {
        private:
            const Graph_t & _graph;
        public:
            YVar(const Graph_t & graph): VarType(lemon::countNodes(graph), 0, OSI_Builder::INFTY, false), _graph(graph) {}
            int id(Graph_t::Node u) { const int id = _graph.id(u);
                assert(id >=0 && id < _number); return _offset + id; }
    };


    void name_variables(OsiClpSolverInterface * solver, const Graph_t & graph, XVar & x_var, YVar & y_var) {
        auto node_str = [&graph] (Graph_t::Node v) { return std::to_string(graph.id(v)); };
        auto arc_str = [&graph, &node_str] (Graph_t::Arc a) { return "(" + node_str(graph.source(a)) + "," + node_str(graph.target(a)) + ")"; };
        // XVar
        for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) 
            solver->setColName(x_var.id(a), "x_" + arc_str(a));
        // YVar
        for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) 
            solver->setColName(y_var.id(u), "y_" + node_str(u));
    }
}

using namespace Solvers::PL_Reff;

static std::string name() {
    return "pl_reff"; 
}

int main() {
    const int log_level = 1;

    Graph_t graph;
    Graph_t::ArcMap<double> resistance(graph, 1);
    
    const Graph_t::Node a = graph.addNode();
    const Graph_t::Node b = graph.addNode();

    const Graph_t::Node c = graph.addNode();
    const Graph_t::Node d = graph.addNode();
    const Graph_t::Node e = graph.addNode();

    resistance[graph.addArc(d, a)] = 2;
    resistance[graph.addArc(d, b)] = 3;
    resistance[graph.addArc(a, c)] = 1.5;
    resistance[graph.addArc(b, c)] = 1;
    resistance[graph.addArc(d, c)] = 2;
    resistance[graph.addArc(a, b)] = 5;
    resistance[graph.addArc(c, e)] = 1;


    const Graph_t::Node s = b;
    const Graph_t::Node t = b;
    const int n = lemon::countNodes(graph);
    
    std::chrono::time_point<std::chrono::high_resolution_clock> last_time, current_time;
    

    XVar x_var(graph);
    YVar y_var(graph);

    OSI_Builder solver_builder = OSI_Builder();

    solver_builder.addVarType(&x_var);
    solver_builder.addVarType(&y_var);
    solver_builder.init();
    
    if(log_level > 0)
        std::cout << name() << ": Start filling solver : " << solver_builder.getNbVars() << " variables" << std::endl;
    last_time = std::chrono::high_resolution_clock::now();



    ////////////////////////////////////////////////////////////////////////
    // Columns : Objective
    ////////////////////
    solver_builder.setObjective(y_var.id(s), n-1);
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        if(v == s) continue;
        solver_builder.setObjective(y_var.id(v), -1);
    }
    ////////////////////////////////////////////////////////////////////////
    // Rows : Constraints
    ////////////////////
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        for(Graph_t::InArcIt e(graph, v); e != lemon::INVALID; ++e)
            solver_builder.buffEntry(x_var.id(e), -1);
        for(Graph_t::OutArcIt e(graph, v); e != lemon::INVALID; ++e)
            solver_builder.buffEntry(x_var.id(e), 1);

        const double bound = (v == s) ? (n-1)/(double)n : -1/(double)n;     // const double bound = (v == s) ? 1 : (v == t) ? -1 : 0;
        solver_builder.pushRow(bound, bound);
    }
    for(Graph_t::ArcIt e(graph); e != lemon::INVALID; ++e) {
        Graph_t::Node u = graph.source(e);
        Graph_t::Node v = graph.target(e);

        solver_builder.buffEntry(y_var.id(u), 1);
        solver_builder.buffEntry(y_var.id(v), -1);
        solver_builder.buffEntry(x_var.id(e), -resistance[e]);

        solver_builder.pushRow(0, 0);
    }
    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////


    
    current_time = std::chrono::high_resolution_clock::now();
    if(log_level >= 1) {
        int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-last_time).count();
        std::cout << name() << ": Complete filling solver : " << solver_builder.getNbConstraints() << " constraints in " << time_ms << " ms" << std::endl;
        std::cout << name() << ": Start solving" << std::endl;
    }
    last_time = current_time;

    OsiClpSolverInterface * solver = solver_builder.buildSolver(OSI_Builder::MIN);

    if(log_level >= 1) {
        name_variables(solver, graph, x_var, y_var);
        solver->writeLp("pl_reff");
    }

    CbcModel model(*solver);
          
    ////////////////////////////////////////////////////////////////////////
    // Compute
    ////////////////////
    model.branchAndBound();
    
    const double * var_solution = model.bestSolution();
    if(var_solution == nullptr) {
        std::cerr << name() << ": Fail" << std::endl;
        return EXIT_FAILURE;
    }

    current_time = std::chrono::high_resolution_clock::now();
    int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-last_time).count();

    if(log_level >= 1) {
        std::cout << name() << ": Complete solving : " << time_ms << " ms" << std::endl;
        std::cout << name() << ": obj : " << model.getObjValue() << std::endl;
    }

    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        const int id_u = graph.id(s);
        const int id_v = graph.id(v);

        const double Ruv = var_solution[y_var.id(s)] - var_solution[y_var.id(v)];
        
        std::cout << "PL R( "<< id_u << " , " << id_v << " ) : " << Ruv << std::endl;
    }

    std::cout << std::endl;

    getReff(graph, resistance, s);

    delete solver;

    return EXIT_SUCCESS;
}