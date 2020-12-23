#include "solvers/bogo.hpp"

#include "utils/random_chooser.hpp"

Solution * Solvers::Bogo::solve(const Landscape & landscape, const RestorationPlan<Landscape>& plan, const double B) const {
    const int seed = params.at("seed")->getInt();
    Chrono chrono;

    RandomChooser<RestorationPlan<Landscape>::Option> option_chooser(seed);
    for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i)
        option_chooser.add(i, 1);

    Solution * solution = new Solution(landscape, plan);
    double purschaised = 0.0;
    while(option_chooser.canPick()) {
        RestorationPlan<Landscape>::Option i = option_chooser.pick();
        const double cost = plan.getCost(i);
        if(purschaised + cost > B) continue;
        purschaised += cost;
        solution->add(i);
    }

    solution->setComputeTimeMs(chrono.timeMs());
    
    return solution;
}