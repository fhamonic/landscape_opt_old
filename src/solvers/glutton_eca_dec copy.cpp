#include "solvers/glutton_eca_dec.hpp"

Solution * Solvers::Glutton_ECA_Dec::solve(const Landscape & landscape, const RestorationPlan<Landscape>& plan, const double B) const {
    const int log_level = params.at("log")->getInt();
    const bool parallel = params.at("parallel")->getBool();
    Chrono chrono;
 
    Solution * solution = new Solution(landscape, plan);

    const Graph_t & graph = landscape.getNetwork();

    std::vector<RestorationPlan<Landscape>::Option> options;
    double purchaised = 0.0;
    for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i) {
        purchaised += plan.getCost(i);
        solution->add(i);
        options.push_back(i);
    }

    double prec_eca = ECA::get().eval_solution(landscape, plan, *solution);
    if(log_level > 1) {
        std::cout << "base purchaised: " << purchaised << std::endl;
        std::cout << "base ECA: " << prec_eca << std::endl;
    }

    auto min_option = [] (std::pair<double, RestorationPlan<Landscape>::Option> p1, std::pair<double, RestorationPlan<Landscape>::Option> p2) {
        return (p1.first < p2.first) ? p1 : p2;
    };
    auto compute_option = [&landscape, &plan, &prec_eca, solution] (RestorationPlan<Landscape>::Option option) {
        DecoredLandscape decored_landscape(landscape);
        
        for(RestorationPlan<Landscape>::Option i=0; i<plan.getNbOptions(); ++i) {
            if(i == option) continue;
            decored_landscape.apply(plan, i, solution->getCoef(i));
        }
        const double eca = ECA::get().eval(decored_landscape);
        const double ratio = (prec_eca - eca) / plan.getCost(option);

        return std::pair<double, RestorationPlan<Landscape>::Option>(ratio, option);
    };
    while(purchaised > B) {
        std::pair<double, RestorationPlan<Landscape>::Option> worst = parallel ?
                std::transform_reduce(std::execution::par, options.begin(), options.end(),
                        std::make_pair(std::numeric_limits<double>::max(), -1), min_option, compute_option) :
                std::transform_reduce(std::execution::seq, options.begin(), options.end(),
                        std::make_pair(std::numeric_limits<double>::max(), -1), min_option, compute_option);        

        const double worst_ratio = worst.first;
        RestorationPlan<Landscape>::Option worst_option = worst.second;
        
        if(worst_option == -1) break;

        options.erase(std::find(options.begin(), options.end(), worst_option));

        const double worst_option_cost = plan.getCost(worst_option);
        solution->remove(worst_option);
        purchaised -= worst_option_cost;
        prec_eca -= worst_ratio * worst_option_cost;

        if(log_level > 1) {
            std::cout << "remove option: " << worst_option_cost << std::endl;
            if(log_level > 2) {
                for(auto const& [u, quality_gain] : plan.getNodes(worst_option))
                    std::cout << "\tn " << graph.id(u) << std::endl;
                for(auto const& [a, restored_probability] : plan.getArcs(worst_option)) {
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