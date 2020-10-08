#ifndef PL_ECA_2_Solver_HPP
#define PL_ECA_2_Solver_HPP

#include "solvers/concept/solver.hpp"
#include "indices/eca.hpp"
#include "helper.hpp"

#include "osi_builder.hpp"

#include "coin/CbcModel.hpp"

namespace Solvers {
    class PL_ECA_2 : public concepts::Solver {
        public:
            PL_ECA_2() {
                params["log"] = new IntParam(0);
                params["threads"] = new IntParam(1);
                params["relaxed"] = new IntParam(0);
                params["timeout"] = new IntParam(3600);
                params["fortest"] = new IntParam(0);
            }

            PL_ECA_2 & setLogLevel(int log_level) {
                params["log"]->set(log_level);
                return *this;
            }
            PL_ECA_2 & setNbThreads(int nb_threads) {
                params["threads"]->set(nb_threads);
                return *this;
            }
            PL_ECA_2 & setRelaxed(bool relaxed) {
                params["relaxed"]->set(relaxed);
                return *this;
            }
            PL_ECA_2 & setTimeout(int seconds) {
                params["timeout"]->set(seconds);
                return *this;
            } 
            PL_ECA_2 & setFortest(int fortest) {
                params["fortest"]->set(fortest);
                return *this;
            }       

            Solution * solve(const Landscape & landscape, const RestorationPlan & options, const double B) const;

            const std::string name() const { return "pl_eca_2"; } 
    };
}


#endif // PL_ECA_SOLVER_HPP