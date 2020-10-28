#include "solvers/glutton_eca_inc.hpp"

Solution * Solvers::Glutton_ECA_Inc::solve(const Landscape & landscape, const RestorationPlan & plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const bool parallel = params.at("parallel")->getBool();
    Chrono chrono;
 
    Solution * solution = new Solution(landscape, plan);

    const Graph_t & graph = landscape.getNetwork();

    std::vector<const RestorationPlan::Option> options;
    double purchaised = 0.0;
    
    for(RestorationPlan::Option i=0; i<plan.getNbOptions(); ++i)
        options.push_back(i);
    
    double prec_eca = ECA::get().eval(landscape);
    if(log_level > 1) {
        std::cout << "base ECA: " << prec_eca << std::endl;
    }

    auto max_option = [](std::pair<double,const RestorationPlan::Option> p1, std::pair<double,const RestorationPlan::Option> p2) {
        return (p1.first > p2.first) ? p1 : p2;
    };
    auto compute_option = [&landscape, &plan, &prec_eca, solution] (const RestorationPlan::Option option) {
        DecoredLandscape decored_landscape(landscape);
        for(RestorationPlan::Option i=0; i<plan.getNbOptions(); ++i) {
            decored_landscape.apply(plan, i, solution->getCoef(i));
        }
        decored_landscape.apply(plan, option);
        const double eca = ECA::get().eval(decored_landscape);
        const double ratio = (eca - prec_eca) / plan.getCost(option);

        return std::pair<double, const RestorationPlan::Option>(ratio, option);
    };
    while(!options.empty()) {
        std::pair<double, const RestorationPlan::Option> best = parallel ?
                std::transform_reduce(std::execution::par, options.begin(), options.end(),
                        std::make_pair(0.0, (const RestorationPlan::Option) nullptr), max_option, compute_option) :
                std::transform_reduce(std::execution::seq, options.begin(), options.end(),
                        std::make_pair(0.0, (const RestorationPlan::Option) nullptr), max_option, compute_option);    

        const double best_ratio = best.first;
        const RestorationPlan::Option best_option = best.second;
        const double best_option_cost = plan.getCost(best_option);

        options.erase(std::find(options.begin(), options.end(), best_option));

        if(purchaised + best_option_cost > B)
            break;

        solution->add(best_option);
        purchaised += best_option_cost;
        prec_eca += best_ratio * best_option_cost;

        if(log_level > 1) {
            std::cout << "add option: " << best_option_cost << std::endl;
            if(log_level > 2) {
                for(auto const& [u, quality_gain] : plan.getNodes(best_option))
                    std::cout << "\tn " << graph.id(u) << std::endl;
                for(auto const& [a, restored_probability] : plan.getArcs(best_option)) {
                    Graph_t::Node source = graph.source(a);
                    Graph_t::Node target = graph.target(a);
                    std::cout << "\ta " << " " << graph.id(source) << " " << graph.id(target) << std::endl;
                }
            }
            std::cout << "current purchaised: " << purchaised << std::endl;
            std::cout << "current ECA: " << prec_eca << std::endl;
            std::cout << "remaining : " << options.size() << std::endl;
        }
    }

    solution->setComputeTimeMs(chrono.timeMs());
    solution->obj = prec_eca;

    return solution;
}