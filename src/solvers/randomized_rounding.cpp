#include "solvers/randomized_rounding.hpp"

static Solution * job(const Landscape & landscape, const RestorationPlan<Landscape>& plan, const double B, const Solution * relaxed_solution, int nb_draws) {
    RandomChooser<RestorationPlan<Landscape>::Option> option_chooser;
    for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i) {
        const double coef = relaxed_solution->getCoef(i);
        if(coef == 0) continue;
        option_chooser.add(i, coef);
    }

    Solution * best_solution = new Solution(landscape, plan);

    std::vector<RestorationPlan<Landscape>::Option> purschaised_options;
    double purschaised;
    DecoredLandscape decored_landscape(landscape);
    double best_eca = 0.0;

    for(int i=0; i<nb_draws; i++) {
        option_chooser.reset();
        purschaised_options.clear();
        purschaised = 0.0;
        decored_landscape.reset();
        while(option_chooser.canPick()) {
            RestorationPlan<Landscape>::Option option = option_chooser.pick();
            if(purschaised + plan.getCost(option) > B) continue;
            purschaised_options.push_back(option);
            purschaised += plan.getCost(option);

            decored_landscape.apply(plan, option);
        }
        double eca = ECA::get().eval(decored_landscape);

        if(eca > best_eca) {
            for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i)
                best_solution->set(i, 0);
            for(RestorationPlan<Landscape>::Option option : purschaised_options)
                best_solution->set(option, 1);

            best_eca = eca;
        }
    }

    best_solution->obj = best_eca;

    return best_solution;
}

Solution * Solvers::Randomized_Rounding_ECA::solve(const Landscape & landscape, const RestorationPlan<Landscape>& plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const int nb_draws = params.at("draws")->getInt();
    const bool parallel = params.at("parallel")->getBool();
    Chrono chrono;
    
    Solvers::PL_ECA_3 pl_eca_3;
    pl_eca_3.setLogLevel(log_level).setRelaxed(1);

    Solution * relaxed_solution = pl_eca_3.solve(landscape, plan, B);

    // //for debug
    // std::cout << std::endl;
    // for(auto option_pair : relaxed_solution->getOptionCoefs()) {
    //     RestorationPlan<Landscape>::Option* option = option_pair.first;
    //     const double coef = option_pair.second;
    //     std::cout << coef << " ";
    // }
    // std::cout << std::endl;

    Solution * best_solution = nullptr;
    if(parallel) {
        const int nb_threads = std::thread::hardware_concurrency();
        const int nb_draws_per_thread = nb_draws / nb_threads + (nb_draws % nb_threads > 0 ? 1 : 0);
        std::vector<Solution *> v(nb_threads);
        std::generate(std::execution::par, v.begin(), v.end(), [&](){ return job(landscape, plan, B, relaxed_solution, nb_draws_per_thread); });

        best_solution = v[0];
        for(int i=1; i<nb_threads; i++) {
            Solution * next_solution = v[i];
            if(best_solution->obj < next_solution->obj)
                std::swap(best_solution, next_solution);
            delete next_solution;
        }
    } else {
        best_solution = job(landscape, plan, B, relaxed_solution, nb_draws);
    }

    best_solution->setComputeTimeMs(chrono.timeMs());

    return best_solution;
}