#include "solvers/bogo.hpp"

#include "random_chooser.hpp"

Solution * Solvers::Bogo::solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const {
    const int seed = params.at("seed")->getInt();
    
    RandomChooser<const RestorationPlan::Option*> option_chooser(seed);
    for(RestorationPlan::Option * option : plan.options())
        option_chooser.add(option, 1);

    Solution * solution = new Solution(landscape, plan);
    double purschaised = 0.0;
    while(option_chooser.canPick()) {
        const RestorationPlan::Option * option = option_chooser.pick();
        if(purschaised + option->getCost() > B)
            continue;
        purschaised += option->getCost();
        solution->add(option);
    }

    return solution;
}