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
    using Option = int;

private:
    std::vector<double> options_costs;
    std::vector<std::pair<MutableLandscape, RestorationPlan<MutableLandscape>>> cases;

public:
    Instance() {}


    /**
     * @brief add an option of cost **c** and returns its id
     * @param c cost
     * @return Option
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    Option addOption(double c) noexcept {
        options_costs.push_back(c);
        return options_costs.size() - 1;
    }

    /**
     * @brief Get the number of options
     * @return int
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    int getNbOptions() const noexcept { return options_costs.size(); }

    /**
     * @brief Return true if the restoration plan contains an
     *      option of id **i**
     * @param i - Option
     * @return bool
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    bool contains(Option i) const noexcept {
        return i >= 0 && i < getNbOptions();
    }

    /**
     * @brief Set the cost of option **i**
     * @param i Option
     * @param c cost
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    void setCost(Option i, double cost) noexcept { options_costs[i] = cost; }

    /**
     * @brief Get the cost of option **i**
     * @param i Option
     * @return double
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    double getCost(Option i) const noexcept { return options_costs[i]; }
};

#endif  // RESTORATION_PLAN_2_HPP