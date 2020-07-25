#ifndef PL_MARKOV_SOLVER_HPP
#define PL_MARKOV_SOLVER_HPP

#include <cmath>

#include "solvers/concept/solver.hpp"
// #include "helper.hpp"

//#include "Eigen/SparseCholesky"
#include "Eigen/SparseLU"

#include "osi_builder.hpp"

#include "coin/CbcModel.hpp"
#include "coin/OsiClpSolverInterface.hpp"

class PL_Markov_Solver : public concepts::Solver {
    public:
        PL_Markov_Solver() {
            params["log"] = new IntParam(0);
            params["threads"] = new IntParam(1);
            params["relaxed"] = new IntParam(0);
        }

        PL_Markov_Solver & setLogLevel(int log_level) {
            params["log"]->set(log_level);
            return *this;
        }
        PL_Markov_Solver & setNbThreads(int nb_threads) {
            params["threads"]->set(nb_threads);
            return *this;
        }
        PL_Markov_Solver & setRelaxed(bool relaxed) {
            params["relaxed"]->set(relaxed);
            return *this;
        }       

        Graph_t::ArcMap<std::map<Graph_t::Arc, double>> * getRhoMap(const Landscape & landscape) const;
        Solution * solve(const Landscape & landscape, const RestorationPlan & plans, const double B) const;

        const std::string name() const { return "pl_markov"; } 
};

#endif // PL_MARKOV_SOLVER_HPP