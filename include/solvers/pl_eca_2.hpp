#ifndef PL_ECA_2_SOLVER_HPP
#define PL_ECA_2_SOLVER_HPP

#include "indices/eca.hpp"
#include "solvers/concept/solver.hpp"

#include "utils/osi_builder.hpp"

namespace Solvers {
class PL_ECA_2 : public concepts::Solver {
public:
    PL_ECA_2() {
        params["log"] = new IntParam(0);
        params["relaxed"] = new IntParam(0);
        params["timeout"] = new IntParam(0);
    }

    PL_ECA_2 & setLogLevel(int log_level) {
        params["log"]->set(log_level);
        return *this;
    }
    PL_ECA_2 & setRelaxed(bool relaxed) {
        params["relaxed"]->set(relaxed);
        return *this;
    }
    PL_ECA_2 & setTimeout(int fortest) {
        params["timeout"]->set(fortest);
        return *this;
    }

    Solution solve(const MutableLandscape & landscape,
                   const RestorationPlan<MutableLandscape> & options,
                   const double B) const;

    const std::string name() const { return "pl_eca_2"; }

    double eval(const MutableLandscape & landscape,
                const RestorationPlan<MutableLandscape> & plan, const double B,
                const Solution & solution) const;
};
}  // namespace Solvers

#endif  // PL_ECA_2_SOLVER_HPP