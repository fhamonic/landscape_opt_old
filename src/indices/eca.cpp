#include "indices/eca.hpp"

ECA ECA::singleton;

ECA::ECA() {}
ECA::~ECA() {}

double ECA::eval_solution(const Landscape & landscape, const RestorationPlan & plan, const Solution & solution) const {
    DecoredLandscape decored_landscape(landscape);
    for(RestorationPlan::Option i=0; i<plan.getNbOptions(); ++i)
        decored_landscape.apply(plan, i, solution.getCoef(i));
    double value = ECA::get().eval(decored_landscape);
    return value;
}