#include "helper.hpp"

bool is_probability(const double p) {
    return (p == p) && (p >= 0) && (p <= 1);
}

void Helper::assert_well_formed(const MutableLandscape & landscape, const RestorationPlan<MutableLandscape>& plan) {
    const MutableLandscape::Graph & graph = landscape.getNetwork();

    for(MutableLandscape::NodeIt v(graph); v != lemon::INVALID; ++v) { (void)v;
        assert(landscape.getQuality(v)==landscape.getQuality(v) && landscape.getQuality(v) >= 0); }
    for(MutableLandscape::ArcIt a(graph); a != lemon::INVALID; ++a) { (void)a;
        assert(is_probability(landscape.getProbability(a))); }

    for(MutableLandscape::NodeIt v(graph); v != lemon::INVALID; ++v) {
        for(auto const & e : plan[v]) { (void)e;
            assert(plan.contains(e.option));
            assert(e.quality_gain > 0.0);
        }
    }
    for(MutableLandscape::ArcIt a(graph); a != lemon::INVALID; ++a) {
        for(auto const & e : plan[a]) { (void)e;
            assert(plan.contains(e.option));
            assert(is_probability(e.restored_probability));
            if(e.restored_probability <= landscape.getProbability(a))
                std::cout << e.restored_probability << " <= " << landscape.getProbability(a) << std::endl;
            assert(e.restored_probability > landscape.getProbability(a));
        }
    }
}