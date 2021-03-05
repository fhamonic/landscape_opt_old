#include "solvers/naive_eca_dec.hpp"

Solution * Solvers::Naive_ECA_Dec::solve(const Landscape & landscape, const RestorationPlan<Landscape>& plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const bool parallel = params.at("parallel")->getBool();
    Chrono chrono;
    
    Solution * solution = new Solution(landscape, plan);

    std::vector<RestorationPlan<Landscape>::Option> free_options;
    std::vector<std::pair<double, RestorationPlan<Landscape>::Option>> ratio_free_options;

    std::vector<RestorationPlan<Landscape>::Option> options;
    std::vector<std::pair<double, RestorationPlan<Landscape>::Option>> ratio_options;

    double purchaised = 0.0;
    for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i) {
        purchaised += plan.getCost(i);
        solution->add(i);
        options.push_back(i);
    }

    double prec_eca = ECA::get().eval_solution(landscape, plan, *solution);
    if(log_level > 1) {
        std::cout << "base purchased: " << purchaised << std::endl;
        std::cout << "base ECA: " << prec_eca << std::endl;
    }

    ratio_options.resize(options.size());

    auto compute_dec = [&landscape, &plan, &options, prec_eca] (RestorationPlan<Landscape>::Option option) {
        DecoredLandscape decored_landscape(landscape);
        for(RestorationPlan<Landscape>::Option it_option : options) {
            if(it_option == option)
                continue;
            decored_landscape.apply(plan, it_option);
        }
        const double eca = ECA::get().eval(decored_landscape);
        const double ratio = (prec_eca - eca) / plan.getCost(option);
        return std::make_pair(ratio, option);
    };
    if(parallel) std::transform(std::execution::par, options.begin(), options.end(), ratio_options.begin(), compute_dec);
    else std::transform(std::execution::seq, options.begin(), options.end(), ratio_options.begin(), compute_dec);

    std::sort(ratio_options.begin(), ratio_options.end(), [](std::pair<double, RestorationPlan<Landscape>::Option> & e1, std::pair<double, RestorationPlan<Landscape>::Option> & e2) {
        return e1.first < e2.first;
    });

    for(std::pair<double, RestorationPlan<Landscape>::Option> elem : ratio_options) {
        if(purchaised <= B) break;
        purchaised -= plan.getCost(elem.second);
        solution->remove(elem.second);
        free_options.push_back(elem.second);
        if(log_level > 1)
            std::cout << "remove ratio: " << elem.first << "\t option: " << elem.second << "\t purchaised: " << purchaised << std::endl;
    }


    prec_eca = ECA::get().eval_solution(landscape, plan, *solution);

    ratio_free_options.resize(free_options.size());

    auto compute_inc = [&landscape, &plan, &solution, prec_eca] (RestorationPlan<Landscape>::Option option) {
        DecoredLandscape decored_landscape(landscape);
        for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i)
            decored_landscape.apply(plan, i, solution->getCoef(i));
        decored_landscape.apply(plan, option);
        const double eca = ECA::get().eval(decored_landscape);
        const double ratio = (eca - prec_eca) / plan.getCost(option);
        return std::make_pair(ratio, option);
    };
    if(parallel) std::transform(std::execution::par, free_options.begin(), free_options.end(), ratio_free_options.begin(), compute_inc);
    else std::transform(std::execution::seq, free_options.begin(), free_options.end(), ratio_free_options.begin(), compute_inc);

    std::sort(ratio_free_options.begin(), ratio_free_options.end(), [](std::pair<double, RestorationPlan<Landscape>::Option> & e1, std::pair<double, RestorationPlan<Landscape>::Option> & e2) {
        return e1.first > e2.first;
    });

    if(log_level > 1)
        std::cout << ratio_free_options.size() << std::endl;

    for(std::pair<double, RestorationPlan<Landscape>::Option> elem : ratio_free_options) {
        const double cost = plan.getCost(elem.second);
        if(log_level > 1)
            std::cout << "evaluate inc ratio: " << elem.first << "\t option: " << elem.second << "\t cost: " << cost << std::endl;
        if(purchaised + cost > B) continue;
        purchaised += cost;
        solution->add(elem.second);
        if(log_level > 1)
            std::cout << "add ratio: " << elem.first << "\t option: " << elem.second << "\t purchaised: " << purchaised << std::endl;
    }


    solution->setComputeTimeMs(chrono.timeMs());

    return solution;
}