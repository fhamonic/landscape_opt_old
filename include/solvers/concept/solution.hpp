#ifndef SOLUTION_HPP
#define SOLUTION_HPP

#include <cassert> // assert

#include "landscape/mutable_landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"

class Solution {
public:
    int nb_vars;
    int nb_constraints;
    int nb_elems;
    double obj;

private:
    // reference wrapper needed for default construct operations
    std::reference_wrapper<const MutableLandscape> landscape;
    std::reference_wrapper<const RestorationPlan<MutableLandscape>> plan;

    std::vector<double> coefs;

    int compute_time_ms;
public:
    Solution(const MutableLandscape & landscape, const RestorationPlan<MutableLandscape>& plan)
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

    const RestorationPlan<MutableLandscape>& getPlan() const { return plan; };

    bool contains(RestorationPlan<MutableLandscape>::Option option) const {
        return coefs.at(option) > std::numeric_limits<double>::epsilon();
    }

    void set(RestorationPlan<MutableLandscape>::Option option, double coef) { coefs[option] = coef; }
    void add(RestorationPlan<MutableLandscape>::Option option) { set(option, 1.0); }
    void remove(RestorationPlan<MutableLandscape>::Option option) { set(option, 0.0); }

    const std::vector<double> & getCoefs() const { return coefs; }

    void setComputeTimeMs(int time_ms) { compute_time_ms = time_ms; }
    int getComputeTimeMs() const { return compute_time_ms; }

    double getCost() const {
        double sum = 0;
        for(RestorationPlan<MutableLandscape>::Option i=0; i<plan.get().getNbOptions(); ++i)
            sum += plan.get().getCost(i) * coefs[i];   
        return sum;
    }

    double getCoef(RestorationPlan<MutableLandscape>::Option i) const { return coefs[i]; }
    double operator[](RestorationPlan<MutableLandscape>::Option i) const { return getCoef(i); }
};

#endif //SOLUTION_HPP