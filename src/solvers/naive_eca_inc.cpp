#include "solvers/naive_eca_inc.hpp"

Solution * Solvers::Naive_ECA_Inc::solve(const Landscape & landscape, const RestorationPlan<Landscape>& plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const bool parallel = params.at("parallel")->getBool();
    Chrono chrono;
    
    Solution * solution = new Solution(landscape, plan);

    double prec_eca = ECA::get().eval(landscape);
    if(log_level > 1) {
        std::cout << "base ECA: " << prec_eca << std::endl;
    }

    std::vector<std::pair<double, RestorationPlan<Landscape>::Option>> ratio_options;

    std::vector<RestorationPlan<Landscape>::Option> options;
    for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i)
        options.push_back(i);
    
    ratio_options.resize(options.size());

    auto compute = [&landscape, &plan, prec_eca] (RestorationPlan<Landscape>::Option option) {
        DecoredLandscape decored_landscape(landscape);
        decored_landscape.apply(plan, option, 1);
        const double eca = ECA::get().eval(decored_landscape);
        const double ratio = (eca - prec_eca) / plan.getCost(option);
        
        return std::make_pair(ratio, option);
    };

    if(parallel) std::transform(std::execution::par, options.begin(), options.end(), ratio_options.begin(), compute);
    else std::transform(std::execution::seq, options.begin(), options.end(), ratio_options.begin(), compute);

    std::sort(ratio_options.begin(), ratio_options.end(), [](std::pair<double, RestorationPlan<Landscape>::Option> & e1, std::pair<double, RestorationPlan<Landscape>::Option> & e2) {
        return e1.first > e2.first;
    });

    double purchaised = 0.0;
    for(std::pair<double, RestorationPlan<Landscape>::Option> elem : ratio_options) {
        const double price = plan.getCost(elem.second);
        if(purchaised + price > B) continue;
        if(log_level > 1)
            std::cout << elem.first << " " << elem.second << std::endl;
        purchaised += price;
        solution->set(elem.second, 1.0);
    }

    solution->setComputeTimeMs(chrono.timeMs());

    return solution;
}