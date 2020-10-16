#include "solvers/naive_eca_dec.hpp"

Solution * Solvers::Naive_ECA_Dec::solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const bool parallel = params.at("parallel")->getBool();
    Helper::Chrono chrono;
    
    Solution * solution = new Solution(landscape, plan);

    double purchaised = 0.0;
    for(RestorationPlan::Option * option : plan.options()) {
        purchaised += option->getCost();
        solution->add(option);
    }

    double prec_eca = ECA::get().eval_solution(landscape, *solution);
    if(log_level > 1) {
        std::cout << "base purchased: " << purchaised << std::endl;
        std::cout << "base ECA: " << prec_eca << std::endl;
    }

    std::vector<std::pair<double, const RestorationPlan::Option *>> ratio_options;
    const std::vector<RestorationPlan::Option*> & options = plan.options();
    
    ratio_options.resize(options.size());

    auto compute = [&landscape, &options, prec_eca] (const RestorationPlan::Option * option) {
        DecoredLandscape decored_landscape(landscape);
        for(RestorationPlan::Option * it_option : options) {
            if(it_option == option)
                continue;
            decored_landscape.apply(it_option, 1);
        }
        const double eca = ECA::get().eval(decored_landscape);
        const double ratio = (prec_eca - eca) / option->getCost();
        return std::make_pair(ratio, option);
    };

    if(parallel) std::transform(std::execution::par, options.begin(), options.end(), ratio_options.begin(), compute);
    else std::transform(std::execution::seq, options.begin(), options.end(), ratio_options.begin(), compute);

    std::sort(ratio_options.begin(), ratio_options.end(), [](std::pair<double, const RestorationPlan::Option *> & e1, std::pair<double, const RestorationPlan::Option *> & e2) {
        return e1.first < e2.first;
    });

    for(std::pair<double, const RestorationPlan::Option *> elem : ratio_options) {
        if(purchaised <= B)
            break;
        purchaised -= elem.second->getCost();;
        solution->remove(elem.second);
        if(log_level > 1)
            std::cout << elem.first << " " << elem.second->getId() << std::endl;
    }

    solution->setComputeTimeMs(chrono.timeMs());

    return solution;
}