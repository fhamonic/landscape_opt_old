#include "solvers/naive_eca_dec.hpp"

Solution * Solvers::Naive_ECA_Dec::solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const bool parallel = params.at("parallel")->getBool();
    Chrono chrono;
    
    Solution * solution = new Solution(landscape, plan);

    double purchaised = 0.0;
    for(RestorationPlan::Option i=0; i<plan.getNbOptions(); ++i) {
        purchaised += plan.getCost(i);
        solution->add(i);
    }

    double prec_eca = ECA::get().eval_solution(landscape, plan, *solution);
    if(log_level > 1) {
        std::cout << "base purchased: " << purchaised << std::endl;
        std::cout << "base ECA: " << prec_eca << std::endl;
    }

    std::vector<std::pair<double, RestorationPlan::Option>> ratio_options;
    
    std::vector<RestorationPlan::Option> options;
    for(RestorationPlan::Option i=0; i<plan.getNbOptions(); ++i)
        options.push_back(i);
    
    ratio_options.resize(options.size());

    auto compute = [&landscape, &plan, &options, prec_eca] (RestorationPlan::Option option) {
        DecoredLandscape decored_landscape(landscape);
        for(RestorationPlan::Option it_option : options) {
            if(it_option == option)
                continue;
            decored_landscape.apply(plan, it_option);
        }
        const double eca = ECA::get().eval(decored_landscape);
        const double ratio = (prec_eca - eca) / plan.getCost(option);
        return std::make_pair(ratio, option);
    };

    if(parallel) std::transform(std::execution::par, options.begin(), options.end(), ratio_options.begin(), compute);
    else std::transform(std::execution::seq, options.begin(), options.end(), ratio_options.begin(), compute);

    std::sort(ratio_options.begin(), ratio_options.end(), [](std::pair<double, RestorationPlan::Option> & e1, std::pair<double, RestorationPlan::Option> & e2) {
        return e1.first < e2.first;
    });

    for(std::pair<double, RestorationPlan::Option> elem : ratio_options) {
        if(purchaised <= B)
            break;
        purchaised -= plan.getCost(elem.second);
        solution->remove(elem.second);
        if(log_level > 1)
            std::cout << elem.first << " " << elem.second << std::endl;
    }

    solution->setComputeTimeMs(chrono.timeMs());

    return solution;
}