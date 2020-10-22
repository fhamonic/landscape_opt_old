#include "solvers/glutton_eca_inc.hpp"

Solution * Solvers::Glutton_ECA_Inc::solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const bool parallel = params.at("parallel")->getBool();
    Chrono chrono;
 
    Solution * solution = new Solution(landscape, plan);

    const Graph_t & graph = landscape.getNetwork();

    std::vector<const RestorationPlan::Option*> options;
    double purchaised = 0.0;
    for(RestorationPlan::Option * option : plan.options()) {
        options.push_back(option);
    }
    
    double prec_eca = ECA::get().eval(landscape);
    if(log_level > 1) {
        std::cout << "base ECA: " << prec_eca << std::endl;
    }

    auto max_option = [](std::pair<double,const RestorationPlan::Option*> p1, std::pair<double,const RestorationPlan::Option*> p2) {
        return (p1.first > p2.first) ? p1 : p2;
    };
    auto compute_option = [&landscape, &prec_eca, solution] (const RestorationPlan::Option* option) {
        DecoredLandscape decored_landscape(landscape);
        for(auto option_pair : solution->getOptionCoefs()) {
            decored_landscape.apply(option_pair.first, option_pair.second);
        }
        decored_landscape.apply(option);
        const double eca = ECA::get().eval(decored_landscape);
        const double ratio = (eca - prec_eca) / option->getCost();

        return std::pair<double, const RestorationPlan::Option*>(ratio, option);
    };
    while(!options.empty()) {
        std::pair<double, const RestorationPlan::Option*> best = parallel ?
                std::transform_reduce(std::execution::par, options.begin(), options.end(),
                        std::make_pair(0.0, (const RestorationPlan::Option*) nullptr), max_option, compute_option) :
                std::transform_reduce(std::execution::seq, options.begin(), options.end(),
                        std::make_pair(0.0, (const RestorationPlan::Option*) nullptr), max_option, compute_option);    

        const double best_ratio = best.first;
        const RestorationPlan::Option * best_option = best.second;

        options.erase(std::find(options.begin(), options.end(), best_option));

        if(purchaised + best_option->getCost() > B)
            break;

        solution->add(best_option);
        purchaised += best_option->getCost();
        prec_eca += best_ratio * best_option->getCost();

        if(log_level > 1) {
            std::cout << "add option: " << best_option->getCost() << std::endl;
            for(Graph_t::Node u : best_option->nodes())
                std::cout << "\tn " << graph.id(u) << std::endl;
            for(Graph_t::Arc a : best_option->arcs()) {
                Graph_t::Node source = graph.source(a);
                Graph_t::Node target = graph.target(a);
                std::cout << "\ta " << " " << graph.id(source) << " " << graph.id(target) << std::endl;
            }
            std::cout << "current purchaised: " << purchaised << std::endl;
            std::cout << "current ECA: " << prec_eca << std::endl;
            //std::cout << "remaining : " << options.size() << std::endl;
        }
    }

    solution->setComputeTimeMs(chrono.timeMs());
    solution->obj = prec_eca;

    return solution;
}