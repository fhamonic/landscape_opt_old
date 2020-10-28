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

        std::vector<double> coefs;

        int compute_time_ms;
    public:
        Solution(const Landscape & landscape, const RestorationPlan & plan) : landscape(landscape), 
                plan(plan), coefs(plan.getNbOptions(), 0.0), compute_time_ms(0) {};
        ~Solution() {};

        const RestorationPlan & getPlan() const { return plan; };

        bool contains(const RestorationPlan::Option option) const {
            return coefs.at(option) > std::numeric_limits<double>::epsilon();
        }

        void set(const RestorationPlan::Option option, double coef) { coefs[option] = coef; }
        void add(const RestorationPlan::Option option) { set(option, 1.0); }
        void remove(const RestorationPlan::Option option) { set(option, 0.0); }

        const std::vector<double> & getCoefs() const { return coefs; }

        void setComputeTimeMs(int time_ms) { compute_time_ms = time_ms; }
        int getComputeTimeMs() const { return compute_time_ms; }

        double getCost() const {
            double sum = 0;
            for(RestorationPlan::Option i=0; i<plan.getNbOptions(); ++i)
                sum += plan.getCost(i) * coefs[i];   
            return sum;
        }

        double getCoef(RestorationPlan::Option i) const { return coefs[i]; }
        double operator[](RestorationPlan::Option i) const { return getCoef(i); }
};

#endif //SOLUTION_HPP