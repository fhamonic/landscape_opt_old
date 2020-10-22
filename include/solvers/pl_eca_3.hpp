#ifndef PL_ECA_3_SOLVER_HPP
#define PL_ECA_3_SOLVER_HPP

#include <algorithm>
#include <execution>

#include "solvers/concept/solver.hpp"
#include "indices/eca.hpp"

#include "utils/osi_builder.hpp"

namespace Solvers {
    class PL_ECA_3 : public concepts::Solver {
        public:
            PL_ECA_3() {
                params["log"] = new IntParam(0);
                params["threads"] = new IntParam(1);
                params["relaxed"] = new IntParam(0);
                params["timeout"] = new IntParam(36000);
                params["fortest"] = new IntParam(0);
            }

            PL_ECA_3 & setLogLevel(int log_level) {
                params["log"]->set(log_level);
                return *this;
            }
            PL_ECA_3 & setNbThreads(int nb_threads) {
                params["threads"]->set(nb_threads);
                return *this;
            }
            PL_ECA_3 & setRelaxed(bool relaxed) {
                params["relaxed"]->set(relaxed);
                return *this;
            }
            PL_ECA_3 & setTimeout(int seconds) {
                params["timeout"]->set(seconds);
                return *this;
            } 
            PL_ECA_3 & setFortest(int fortest) {
                params["fortest"]->set(fortest);
                return *this;
            }            

            Solution * solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const;

            const std::string name() const { return "pl_eca_3"; } 

            double eval(const Landscape & landscape, const RestorationPlan & plan, const double B, const Solution & solution) const;
    };
}

#endif // PL_ECA_SOLVER_HPP