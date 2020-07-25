#ifndef NAIVE_ECA_DEC_SOLVER_HPP
#define NAIVE_ECA_DEC_SOLVER_HPP

#include <execution>
#include <vector>
#include <utility>
	
#include "solvers/concept/solver.hpp"
#include "indices/eca.hpp"

#include "helper.hpp"

namespace Solvers {
    class Naive_ECA_Dec : public concepts::Solver {
        public:
            Naive_ECA_Dec() {
                params["log"] = new IntParam(0);
                params["parallel"] = new IntParam(0);
            }

            Naive_ECA_Dec & setLogLevel(int log_level) {
                params["log"]->set(log_level);
                return *this;
            }

            Naive_ECA_Dec & setParallel(int parallel) {
                params["parallel"]->set(parallel);
                return *this;
            }

            Solution * solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const;

            const std::string name() const { return "naive_eca_dec"; }
    };
}


#endif // NAIVE_ECA_DEC_SOLVER_HPP