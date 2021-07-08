#include "indices/parallel_eca.hpp"

Parallel_ECA Parallel_ECA::singleton;

Parallel_ECA::Parallel_ECA() {}
Parallel_ECA::~Parallel_ECA() {}

double Parallel_ECA::eval_solution(const Landscape & landscape, const RestorationPlan<Landscape>& plan, const Solution & solution) const {
    DecoredLandscape decored_landscape(landscape);
    for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i)
        decored_landscape.apply(plan, i, solution.getCoef(i));
    double value = Parallel_ECA::get().eval(decored_landscape);
    return value;
}