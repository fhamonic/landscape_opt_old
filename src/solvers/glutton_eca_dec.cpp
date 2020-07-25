#include "solvers/glutton_eca_dec.hpp"

Solution * Solvers::Glutton_ECA_Dec::solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const bool parallel = params.at("parallel")->getBool();
    std::chrono::time_point<std::chrono::high_resolution_clock> last_time, current_time;
    last_time = std::chrono::high_resolution_clock::now();
 
    Solution * solution = new Solution(landscape, plan);

    const Graph_t & graph = landscape.getNetwork();

    std::vector<const RestorationPlan::Option*> options;
    double purchaised = 0.0;
    for(RestorationPlan::Option * option : plan.options()) {
        purchaised += option->getCost();
        solution->add(option);
        options.push_back(option);
    }

    double prec_eca = ECA::get().eval(landscape, *solution);
    if(log_level > 1) {
        std::cout << "base purchaised: " << purchaised << std::endl;
        std::cout << "base ECA: " << prec_eca << std::endl;
    }

    auto min_option = [](std::pair<double, const RestorationPlan::Option*> p1, std::pair<double, const RestorationPlan::Option*> p2) {
        return (p1.first < p2.first) ? p1 : p2;
    };
    auto compute_option = [&landscape, &prec_eca, solution] (const RestorationPlan::Option* option) {
        DecoredLandscape decored_landscape(landscape);
        for(auto option_pair : solution->getOptionCoefs()) {
            if(option == option_pair.first)
                continue;
            decored_landscape.apply(option_pair.first, option_pair.second);
        }
        const double eca = ECA::get().eval(decored_landscape);
        const double ratio = (prec_eca - eca) / option->getCost();

        return std::pair<double, const RestorationPlan::Option*>(ratio, option);
    };
    while(purchaised > B) {
        std::pair<double, const RestorationPlan::Option*> worst = parallel ?
                std::transform_reduce(std::execution::par, options.begin(), options.end(),
                        std::make_pair(std::numeric_limits<double>::max(), (const RestorationPlan::Option*) nullptr), min_option, compute_option) :
                std::transform_reduce(std::execution::seq, options.begin(), options.end(),
                        std::make_pair(std::numeric_limits<double>::max(), (const RestorationPlan::Option*) nullptr), min_option, compute_option);        

        const double worst_ratio = worst.first;
        const RestorationPlan::Option * worst_option = worst.second;

        options.erase(std::find(options.begin(), options.end(), worst_option));

        solution->remove(worst_option);
        purchaised -= worst_option->getCost();
        prec_eca -= worst_ratio * worst_option->getCost();

        if(log_level > 1) {
            std::cout << "remove option: " << worst_option->getCost() << std::endl;
            for(Graph_t::Node u : worst_option->nodes())
                std::cout << "\tn " << graph.id(u) << std::endl;
            for(Graph_t::Arc a : worst_option->arcs()) {
                Graph_t::Node source = graph.source(a);
                Graph_t::Node target = graph.target(a);
                std::cout << "\ta " << " " << graph.id(source) << " " << graph.id(target) << std::endl;
            }
            std::cout << "current purchaised: " << purchaised << std::endl;
            std::cout << "current ECA: " << prec_eca << std::endl;
            std::cout << "remaining : " << options.size() << std::endl;
        }
    }

    current_time = std::chrono::high_resolution_clock::now();
    int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-last_time).count();

    solution->setComputeTimeMs(time_ms);
    solution->obj = prec_eca;

    return solution;
}