#include "indices/eca.hpp"

ECA ECA::singleton;

ECA::ECA() {}
ECA::~ECA() {}

double ECA::eval_solution(const Landscape & landscape, const Solution & solution) const {
    DecoredLandscape decored_landscape(landscape);
    for(auto option_pair : solution.getOptionCoefs())
        decored_landscape.apply(option_pair.first, option_pair.second);    
    double value = ECA::get().eval(decored_landscape);
    return value;
}