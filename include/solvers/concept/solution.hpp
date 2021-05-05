#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include <cassert> // assert

#include "landscape/landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"

class Solution {
public:
    int nb_vars;
    int nb_constraints;
    int nb_elems;
    double obj;

private:
    std::reference_wrapper<const Landscape> landscape;
    std::reference_wrapper<const RestorationPlan<Landscape>> plan;

    std::vector<double> coefs;

    int compute_time_ms;
public:
    Solution(const Landscape & landscape, const RestorationPlan<Landscape>& plan)
        : landscape(landscape)
        , plan(plan)
        , coefs(plan.getNbOptions(), 0.0)
        , compute_time_ms(0)
        {};
    Solution(const Solution&) = default; // copy constructor
    Solution(Solution&&) = default; // move constructor
    ~Solution() = default;
    Solution & operator=(const Solution& ) = default; // copy assignment operator
    Solution & operator=(Solution&& s) = default; // move assignment operator

    const RestorationPlan<Landscape>& getPlan() const { return plan; };

    bool contains(RestorationPlan<Landscape>::Option option) const {
        return coefs.at(option) > std::numeric_limits<double>::epsilon();
    }

    void set(RestorationPlan<Landscape>::Option option, double coef) { coefs[option] = coef; }
    void add(RestorationPlan<Landscape>::Option option) { set(option, 1.0); }
    void remove(RestorationPlan<Landscape>::Option option) { set(option, 0.0); }

    const std::vector<double> & getCoefs() const { return coefs; }

    void setComputeTimeMs(int time_ms) { compute_time_ms = time_ms; }
    int getComputeTimeMs() const { return compute_time_ms; }

    double getCost() const {
        double sum = 0;
        for(RestorationPlan<Landscape>::Option i=0; i<plan.get().getNbOptions(); ++i)
            sum += plan.get().getCost(i) * coefs[i];   
        return sum;
    }

    double getCoef(RestorationPlan<Landscape>::Option i) const { return coefs[i]; }
    double operator[](RestorationPlan<Landscape>::Option i) const { return getCoef(i); }
};

#endif //SOLUTION_HPP