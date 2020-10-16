#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include <cassert> // assert

#include "landscape/landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"

class Solution {
    public:
        int nb_vars;
        int nb_constraints;
        double obj;

    private:
        const Landscape & landscape;
        const RestorationPlan & plan;

        // TODO replace by vector<std::pair<const RestorationPlan::Option *, double>>
        std::map<const RestorationPlan::Option *, double> optionsCoefs;

        int compute_time_ms;
    public:
        Solution(const Landscape & landscape, const RestorationPlan & plan) : landscape(landscape), plan(plan), compute_time_ms(0) {
            for(RestorationPlan::Option * option : plan.options())
                set(option, 0.0);
        };
        ~Solution() {};

        const RestorationPlan & getPlan() const { return plan; };

        bool contains(const RestorationPlan::Option * option) const {
            return optionsCoefs.at(option) > __DBL_EPSILON__;
        }

        void set(const RestorationPlan::Option * option, double coef) {
            optionsCoefs[option] = coef;
        }
        void add(const RestorationPlan::Option * option) { set(option, 1.0); }
        void remove(const RestorationPlan::Option * option) { set(option, 0.0); }

        const std::map<const RestorationPlan::Option*, double> & getOptionCoefs() const {
            return optionsCoefs;
        }

        void setComputeTimeMs(int time_ms) { compute_time_ms = time_ms; }
        int getComputeTimeMs() const { return compute_time_ms; }

        double getCost() const {
            double sum = 0;

            for(auto option_pair : optionsCoefs) {
                const RestorationPlan::Option * option = option_pair.first;
                double coef = option_pair.second;

                sum += option->getCost() * coef;
            }        
            return sum;
        };
};

#endif //SOLUTION_HPP