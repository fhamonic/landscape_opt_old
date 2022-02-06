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
#include <unordered_map>

#include <parallel_hashmap/phmap.h>

#include "landscape/mutable_landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"

using Option = int;

struct InstanceCase {
    double coef;
    void * graph;

    std::vector<double> node_quality_map;
    std::vector<double> arc_probability_map;

    std::vector<std::string> node_names;
    phmap::node_hash_map<std::string, unsigned int> node_name_to_id_map;
    std::vector<std::string> arc_names;
    phmap::node_hash_map<std::string, unsigned int> arc_name_to_id_map;

    std::vector<std::vector<std::pair<Option, double>>> node_options_map;
    std::vector<std::vector<std::pair<Option, double>>> arc_options_map;
};

class Instance2 {
private:
    std::vector<std::string> options_names;
    std::vector<double> options_costs;
    phmap::node_hash_map<std::string, Option> option_name_to_id_map;

    std::vector<InstanceCase> cases;

public:
    Instance2() = default;

    Option addOption(std::string identifier, double c) noexcept {
        Option i = options_costs.size();
        if(option_name_to_id_map.contains(identifier)) throw "caca";
        options_names.emplace_back(identifier);
        options_costs.emplace_back(c);
        option_name_to_id_map[identifier] = i;
        return i;
    }

    int getNbOptions() const noexcept { return options_costs.size(); }

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