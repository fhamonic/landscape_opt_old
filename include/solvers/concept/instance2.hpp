/**
 * @file restoration_plan.hpp
 * @author François Hamonic (francois.hamonic@gmail.com)
 * @brief Restorationplan class declaration
 * @version 0.3
 * @date 2021-07-18
 *
 * @copyright Copyright (c) 2021
 */
#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include <cassert>
#include <list>
#include <map>
#include <memory>

#include "landscape/mutable_landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"

class Instance {
public:
    std::vector<double> options_costs;
    std::vector<std::pair<MutableLandscape, RestorationPlan<MutableLandscape>>> landscapes;

    Instance() {}
};

#endif  // RESTORATION_PLAN_2_HPP