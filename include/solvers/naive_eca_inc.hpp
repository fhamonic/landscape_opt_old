#ifndef NAIVE_ECA_INC_SOLVER_HPP
#define NAIVE_ECA_INC_SOLVER_HPP

#include <utility>

#include "solvers/concept/solver.hpp"
#include "indices/eca.hpp"

namespace Solvers {
    class Naive_ECA_Inc : public concepts::Solver {
        public:
            Naive_ECA_Inc() {
                params["log"] = new IntParam(0);
                params["parallel"] = new IntParam(0);
            }

            Naive_ECA_Inc & setLogLevel(int log_level) {
                params["log"]->set(log_level);
                return *this;
            }

            Naive_ECA_Inc & setParallel(int parallel) {
                params["parallel"]->set(parallel);
                return *this;
            }

            Solution * solve(const Landscape & landscape, const RestorationPlan<Landscape>& plan, const double B) const;

            const std::string name() const { return "naive_eca_inc"; }
    };
}


#endif // NAIVE_ECA_INC_SOLVER_HPP