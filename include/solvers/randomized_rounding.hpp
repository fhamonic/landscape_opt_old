#ifndef RANDOMIZED_ROUNDING_SOLVER_HPP
#define RANDOMIZED_ROUNDING_SOLVER_HPP

#include "solvers/concept/solver.hpp"

#include <execution>
#include <thread>

#include "solvers/pl_eca_3.hpp"
#include "utils/random_chooser.hpp"

namespace Solvers {
    class Randomized_Rounding_ECA : public concepts::Solver {
        public:
            Randomized_Rounding_ECA() {
                params["log"] = new IntParam(0);
                params["draws"] = new IntParam(0);
                params["parallel"] = new IntParam(0);
            }

            Randomized_Rounding_ECA & setLogLevel(int log_level) {
                params["log"]->set(log_level);
                return *this;
            }
            Randomized_Rounding_ECA & setNbDraws(int log_level) {
                params["draws"]->set(log_level);
                return *this;
            }            
            Randomized_Rounding_ECA & setParallel(int parallel) {
                params["parallel"]->set(parallel);
                return *this;
            }

            Solution * solve(const Landscape & landscape, const RestorationPlan<Landscape>& plans, const double B) const;

            const std::string name() const { return "randomized_rounding"; } 
    };
}

#endif // RANDOMIZED_ROUNDING_SOLVER_HPP