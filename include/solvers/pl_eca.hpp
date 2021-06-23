#ifndef PL_ECA_SOLVER_HPP
#define PL_ECA_SOLVER_HPP

#include "solvers/concept/solver.hpp"
#include "indices/eca.hpp"
#include "helper.hpp"

#include "osi_builder.hpp"

#include "coin/CbcModel.hpp"

#define RESTORED 0
#define DEFAULT 1

#define FIRST 0
#define SECOND 1
#define BOTH 2

class PL_ECA_Solver : public concepts::Solver {
    public:
        PL_ECA_Solver() {
            params["log"] = new IntParam(0);
            params["threads"] = new IntParam(1);
            params["pieces"] = new IntParam(10);
            params["thresold"] = new DoubleParam(0.0);
            params["relaxed"] = new IntParam(0);
            params["timeout"] = new IntParam(3600);
        }

        PL_ECA_Solver & setLogLevel(int log_level) {
            params["log"]->set(log_level);
            return *this;
        }
        PL_ECA_Solver & setNbThreads(int nb_threads) {
            params["threads"]->set(nb_threads);
            return *this;
        }
        PL_ECA_Solver & setPiecewiseNumber(int g) {
            params["pieces"]->set(g);
            return *this;
        }        
        PL_ECA_Solver & setPairThresold(double st_thresold) {
            params["thresold"]->set(st_thresold);
            return *this;
        }
        PL_ECA_Solver & setRelaxed(bool relaxed) {
            params["relaxed"]->set(relaxed);
            return *this;
        }    
        PL_ECA_Solver & setTimeout(int seconds) {
            params["timeout"]->set(seconds);
            return *this;
        }     

        Solution solve(const Landscape & landscape, const RestorationPlan<Landscape>& plan, const double B) const;

        const std::string name() const { return "pl_eca"; }
};

#endif //PL_ECA_SOLVER_HPP