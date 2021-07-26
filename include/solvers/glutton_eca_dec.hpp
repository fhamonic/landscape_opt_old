#ifndef GLUTTON_ECA_DEC_SOLVER_HPP
#define GLUTTON_ECA_DEC_SOLVER_HPP

#include "solvers/concept/solver.hpp"
#include "indices/eca.hpp"

#include <execution>

namespace Solvers {
    class Glutton_ECA_Dec : public concepts::Solver {
        public:
            Glutton_ECA_Dec() {
                params["log"] = new IntParam(0);
                params["parallel"] = new IntParam(0);
            }

            Glutton_ECA_Dec & setLogLevel(int log_level) {
                params["log"]->set(log_level);
                return *this;
            }
            
            Glutton_ECA_Dec & setParallel(int parallel) {
                params["parallel"]->set(parallel);
                return *this;
            }

            Solution solve(const MutableLandscape & landscape, const RestorationPlan<MutableLandscape> & plan, const double B) const;

            const std::string name() const { return "glutton_eca_dec"; } 
    };
}

#endif // GLUTTON_ECA_SOLVER_HPP