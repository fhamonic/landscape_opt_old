#include "solvers/bogo.hpp"

#include "utils/random_chooser.hpp"

Solution Solvers::Bogo::solve(const MutableLandscape & landscape,
                              const RestorationPlan<MutableLandscape> & plan,
                              const double B) const {
    Solution solution(landscape, plan);
    const int seed = params.at("seed")->getInt();
    Chrono chrono;

    RandomChooser<RestorationPlan<MutableLandscape>::Option> option_chooser(
        seed);
    for(const RestorationPlan<MutableLandscape>::Option i : plan.options())
        option_chooser.add(i, 1);

    double purschaised = 0.0;
    while(option_chooser.canPick()) {
        RestorationPlan<MutableLandscape>::Option i = option_chooser.pick();
        const double cost = plan.getCost(i);
        if(purschaised + cost > B) continue;
        purschaised += cost;
        solution.add(i);
    }

    solution.setComputeTimeMs(chrono.timeMs());

    return solution;
}