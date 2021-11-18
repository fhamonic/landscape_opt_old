#include "solvers/naive_eca_inc.hpp"

Solution Solvers::Naive_ECA_Inc::solve(
    const MutableLandscape & landscape,
    const RestorationPlan<MutableLandscape> & plan, const double B) const {
    Solution solution(landscape, plan);
    const int log_level = params.at("log")->getInt();
    const bool parallel = params.at("parallel")->getBool();
    Chrono chrono;

    const auto nodeOptions = plan.computeNodeOptionsMap();
    const auto arcOptions = plan.computeArcOptionsMap();

    double prec_eca = ECA().eval(landscape);
    if(log_level > 1) {
        std::cout << "base ECA: " << prec_eca << std::endl;
    }

    std::vector<std::pair<double, RestorationPlan<MutableLandscape>::Option>>
        ratio_options;

    std::vector<RestorationPlan<MutableLandscape>::Option> options;
    for(const RestorationPlan<MutableLandscape>::Option i : plan.options())
        options.push_back(i);

    ratio_options.resize(options.size());

    auto compute = [&landscape, &plan, &nodeOptions, &arcOptions, prec_eca](
                       RestorationPlan<MutableLandscape>::Option option) {
        DecoredLandscape<MutableLandscape> decored_landscape(landscape);
        decored_landscape.apply(nodeOptions[option], arcOptions[option]);
        const double eca = ECA().eval(decored_landscape);
        const double ratio = (eca - prec_eca) / plan.getCost(option);

        return std::make_pair(ratio, option);
    };

    if(parallel)
        std::transform(std::execution::par, options.begin(), options.end(),
                       ratio_options.begin(), compute);
    else
        std::transform(std::execution::seq, options.begin(), options.end(),
                       ratio_options.begin(), compute);

    std::sort(
        ratio_options.begin(), ratio_options.end(),
        [](std::pair<double, RestorationPlan<MutableLandscape>::Option> & e1,
           std::pair<double, RestorationPlan<MutableLandscape>::Option> & e2) {
            return e1.first > e2.first;
        });


    for(std::pair<double, RestorationPlan<MutableLandscape>::Option> e :
    ratio_options) {
        std::cout << "troncon " << e.second << " delta " << e.first<< std::endl;
    }

    double purchaised = 0.0;
    for(std::pair<double, RestorationPlan<MutableLandscape>::Option> elem :
        ratio_options) {
        const double price = plan.getCost(elem.second);
        if(purchaised + price > B) continue;
        if(log_level > 1)
            std::cout << elem.first << " " << elem.second << std::endl;
        purchaised += price;
        solution.set(elem.second, 1.0);
        if(log_level > 1)
            std::cout << "add ratio: " << elem.first
                      << "\t option: " << elem.second
                      << "\t purchaised: " << purchaised << std::endl;
    }

    solution.setComputeTimeMs(chrono.timeMs());

    return solution;
}