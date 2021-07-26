#include "helper.hpp"

bool is_probability(const double p) {
    return (p == p) && (p >= 0) && (p <= 1);
}

void Helper::assert_well_formed(const MutableLandscape & landscape, const RestorationPlan<MutableLandscape>& plan) {
    const Graph_t & graph = landscape.getNetwork();

    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) { (void)v;
        assert(landscape.getQuality(v)==landscape.getQuality(v) && landscape.getQuality(v) >= 0); }
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) { (void)a;
        assert(is_probability(landscape.getProbability(a))); }

    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        for(auto const & e : plan[v]) { (void)e;
            assert(plan.contains(e.option));
            assert(e.quality_gain > 0.0);
        }
    }
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
        for(auto const & e : plan[a]) { (void)e;
            assert(plan.contains(e.option));
            assert(is_probability(e.restored_probability));
            assert(e.restored_probability > landscape.getProbability(a));
        }
    }
}