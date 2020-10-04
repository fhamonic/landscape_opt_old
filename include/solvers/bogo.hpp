#ifndef BOGO_SOLVER_HPP
#define BOGO_SOLVER_HPP

#include "solvers/concept/solver.hpp"
 #include "helper.hpp"

namespace Solvers {
    class Bogo : public concepts::Solver {
        public:
            Bogo() {
                params["seed"] = new IntParam(0);
                params["draws"] = new IntParam(0);
            }
            Bogo & setNbDraws(int log_level) {
                params["draws"]->set(log_level);
                return *this;
            }
            Bogo & setSeed(int seed) {
                params["seed"]->set(seed);
                return *this;
            }

            Solution * solve(const Landscape & landscape, const RestorationPlan & plans, const double B) const;

            const std::string name() const { return "bogo"; } 
    };
}

#endif // BOGO_SOLVER_HPP