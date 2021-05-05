#ifndef BOGO_SOLVER_HPP
#define BOGO_SOLVER_HPP

#include "solvers/concept/solver.hpp"

namespace Solvers {
    class Bogo : public concepts::Solver {
        public:
            Bogo() {
                params["seed"] = new IntParam(0);
            }
            Bogo & setSeed(int seed) {
                params["seed"]->set(seed);
                return *this;
            }

            Solution solve(const Landscape & landscape, const RestorationPlan<Landscape>& plans, const double B) const;

            const std::string name() const { return "bogo"; } 
    };
}

#endif // BOGO_SOLVER_HPP