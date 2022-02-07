#ifndef INSTANCE2_HPP
#define INSTANCE2_HPP

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

    Option addOption(std::string identifier, double c) {
        assert(!option_name_to_id_map.contains(identifier));
        Option i = options_costs.size();
        options_names.emplace_back(identifier);
        options_costs.emplace_back(c);
        option_name_to_id_map[identifier] = i;
        return i;
    }

    int getNbOptions() const noexcept { return options_costs.size(); }

    bool containsOption(Option i) const noexcept {
        return i >= 0 && i < getNbOptions();
    }
    bool containsOption(std::string identifier) const noexcept {
        return option_name_to_id_map.contains(identifier);
    }
    void setOptionCost(Option i, double cost) noexcept { options_costs[i] = cost; }
    double getOptionCost(Option i) const noexcept { return options_costs[i]; }
};

#endif  // INSTANCE2_HPP