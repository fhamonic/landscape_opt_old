#include "solvers/naive_eca_inc.hpp"

Solution * Solvers::Naive_ECA_Inc::solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const bool parallel = params.at("parallel")->getBool();
    
    std::chrono::time_point<std::chrono::high_resolution_clock> last_time, current_time;
    last_time = std::chrono::high_resolution_clock::now();
    
    Solution * solution = new Solution(landscape, plan);

    double prec_eca = ECA::get().eval(landscape);
    if(log_level > 1) {
        std::cout << "base ECA: " << prec_eca << std::endl;
    }

    std::vector<std::pair<double, const RestorationPlan::Option *>> ratio_options;
    const std::vector<RestorationPlan::Option*> & options = plan.options();
    
    ratio_options.resize(options.size());

    auto compute = [&landscape, prec_eca] (RestorationPlan::Option * option) {
        DecoredLandscape decored_landscape(landscape);
        decored_landscape.apply(option, 1);
        const double eca = ECA::get().eval(decored_landscape);
        const double ratio = (eca - prec_eca) / option->getCost();
        
        return std::make_pair(ratio, option);
    };

    if(parallel)
        std::transform(std::execution::par, options.begin(), options.end(), ratio_options.begin(), compute);
    else
        std::transform(std::execution::seq, options.begin(), options.end(), ratio_options.begin(), compute);

    std::sort(ratio_options.begin(), ratio_options.end(), [](std::pair<double, const RestorationPlan::Option *> & e1, std::pair<double, const RestorationPlan::Option *> & e2) {
        return e1.first > e2.first;
    });

    double purchaised = 0.0;
    for(std::pair<double, const RestorationPlan::Option *> elem : ratio_options) {
        const double price = elem.second->getCost();
        if(purchaised + price > B)
            continue;

        if(log_level > 1)
            std::cout << elem.first << " " << elem.second->getId() << std::endl;

        purchaised += price;
        solution->set(elem.second, 1.0);
    }

    current_time = std::chrono::high_resolution_clock::now();
    int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-last_time).count();

    solution->setComputeTimeMs(time_ms);

    return solution;
}