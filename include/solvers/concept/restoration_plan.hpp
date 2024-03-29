/**
 * @file restoration_plan.hpp
 * @author François Hamonic (francois.hamonic@gmail.com)
 * @brief Restorationplan class declaration
 * @version 0.3
 * @date 2021-07-27
 *
 * @copyright Copyright (c) 2021
 */
#ifndef RESTORATION_PLAN_HPP
#define RESTORATION_PLAN_HPP

#include <cassert>
#include <list>
#include <map>
#include <memory>
#include <numeric>

#include "landscape/concept/abstract_landscape.hpp"

#include <boost/container/small_vector.hpp>
#include <boost/range/algorithm/find_if.hpp>

#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

/**
 * @brief A generic class for respresenting a landscape restoration plan.
 *
 * This class represents a restoration plan wich is theorically a set of
 * restoration options. Each restoration option have a cost and a set of nodes
 * and arcs that could be enhanced alongside with informations about their
 * weights enhancements. In this class an Option is represented by an int, a
 * vector maps Otions to their costs and each Node/Arc is associated with the
 * list of pairs (option, weight_enhancement) concerning it.
 */
template <typename LS>  // requires concepts::IsLandscape<LS> //c++20
class RestorationPlan {
public:
    using Graph = typename LS::Graph;
    using Node = typename LS::Node;
    using Arc = typename LS::Arc;

    using Option = int;

    struct NodeRestorationElement {
        int id;
        Option option;
        double quality_gain;
        NodeRestorationElement(int i, Option o, double q)
            : id(i), option(o), quality_gain(q) {}
    };
    struct ArcRestorationElement {
        int id;
        Option option;
        double restored_probability;
        ArcRestorationElement(int i, Option o, double p)
            : id(i), option(o), restored_probability(p) {}
    };

    using NodeRestorationsList =
        boost::container::small_vector<NodeRestorationElement, 1>;
    using ArcRestorationsList =
        boost::container::small_vector<ArcRestorationElement, 1>;

    using NodeEnhancements = std::vector<std::pair<Node, double>>;
    using ArcEnhancements = std::vector<std::pair<Arc, double>>;

    using NodeOptionsMap = std::vector<NodeEnhancements>;
    using ArcOptionsMap = std::vector<ArcEnhancements>;

private:
    const LS & _landscape;

    typename Graph::template NodeMap<NodeRestorationsList>
        _qualityRestorationOptions;
    typename Graph::template ArcMap<ArcRestorationsList>
        _probabilityRestorationOptions;

    std::vector<double> _costs;

public:
    RestorationPlan(const LS & l)
        : _landscape(l)
        , _qualityRestorationOptions(l.getNetwork())
        , _probabilityRestorationOptions(l.getNetwork()) {}
    RestorationPlan(const RestorationPlan<LS> & rp)
        : _landscape(rp.getLandscape())
        , _qualityRestorationOptions(_landscape.getNetwork())
        , _probabilityRestorationOptions(_landscape.getNetwork()) {
        throw std::runtime_error(
            "RestorationPlan : non implement copy constructors");
    }
    RestorationPlan(RestorationPlan<LS> && rp)
        : _landscape(rp.getLandscape())
        , _qualityRestorationOptions(_landscape.getNetwork())
        , _probabilityRestorationOptions(_landscape.getNetwork()) {
        throw std::runtime_error(
            "RestorationPlan : non implement copy constructors");
    }
    ~RestorationPlan() {}
    RestorationPlan<LS> & operator=(const RestorationPlan<LS> &) {
        throw std::runtime_error(
            "RestorationPlan : non implement copy constructors");
        return *this;
    };
    RestorationPlan<LS> & operator=(RestorationPlan<LS> &&) {
        throw std::runtime_error(
            "RestorationPlan : non implement copy constructors");
        return *this;
    };

    const LS & getLandscape() const noexcept { return _landscape; }

    /**
     * @brief add an option of cost **c** and returns its id
     * @param c cost
     * @return Option
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    Option addOption(double c) noexcept {
        _costs.push_back(c);
        return _costs.size() - 1;
    }

    /**
     * @brief Get the number of options
     * @return int
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    int getNbOptions() const noexcept { return _costs.size(); }

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
    void setCost(Option i, double cost) noexcept { _costs[i] = cost; }

    /**
     * @brief Get the cost of option **i**
     * @param i Option
     * @return double
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    double getCost(Option i) const noexcept { return _costs[i]; }

    /**
     * Add a restoration enhancement for the node **u** within option **i**.
     * If there is already an enhancement for the node **u** and option **i**,
     * **quality_gain** is added to the enhancement quality gain.
     *
     * @brief Add the node **v** to the option **i**
     * @param i Option
     * @param v Node
     * @param quality_gain - quality gain on **v**
     * @time \f$O(\#options(v))\f$
     * @space \f$O(1)\f$
     */
    void addNode(Option i, Node v, double quality_gain) noexcept {
        assert(contains(i));
        auto & restoration_options = _qualityRestorationOptions[v];
        auto it = boost::find_if(restoration_options,
                                 [i](const auto & p) { return p.option == i; });
        if(it != restoration_options.end()) {
            it->quality_gain += quality_gain;
            return;
        }
        restoration_options.emplace_back(-1, i, quality_gain);
    }

    /**
     * @brief Return true if the option **i** concerns node **v**
     * @param i Option
     * @param v Node
     * @return bool true if the option **i** concerns node **v**
     *      false otherwise
     * @time \f$O(\#options(v))\f$
     * @space \f$O(1)\f$
     */
    bool contains(Option i, Node v) const noexcept {
        assert(contains(i));
        auto & restoration_options = _qualityRestorationOptions[v];
        return std::any_of(restoration_options.begin(),
                           restoration_options.end(),
                           [i](const auto & p) { return p.option == i; });
    }

    /**
     * @brief Remove the node **v** from the option **i**
     * @param i Option
     * @param v Node
     * @time \f$O(\#options(v))\f$
     * @space \f$O(1)\f$
     */
    void removeNode(Option i, Node v) noexcept {
        assert(contains(i));
        auto & restoration_options = _qualityRestorationOptions[v];
        auto it = boost::find_if(restoration_options,
                                 [i](const auto & p) { return p.option == i; });
        if(it == restoration_options.end()) return;
        restoration_options.erase(it);
    }

    /**
     * @brief Test if there is an option concerning the node **v**
     * @param v Node
     * @return bool true if the option **i** concerns node **v**
     *      false otherwise
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    bool contains(Node v) const noexcept {
        return _qualityRestorationOptions[v].size() > 0;
    }

    /**
     * @brief Remove the node **v** from every option
     * @param v Node
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    void removeNode(Node v) noexcept { _qualityRestorationOptions[v].clear(); }

    /**
     * Add a restoration enhancement for the arc **a** within option **i**.
     * If there is already an enhancement for the arc **a** and option **i**,
     * the restored probability is updated to keep the maximum value.
     *
     * @brief Add the arc **a** to the option **i**
     * @param i Option
     * @param a Arc
     * @param restored_probability - restored probability of **a**
     * @time \f$O(\#options(a))\f$
     * @space \f$O(1)\f$
     */
    void addArc(Option i, Arc a, double restored_probability) noexcept {
        assert(contains(i));
        auto & restoration_options = _probabilityRestorationOptions[a];
        auto it = boost::find_if(restoration_options,
                                 [i](const auto & p) { return p.option == i; });
        if(it != restoration_options.end()) {
            it->restored_probability =
                std::max(restored_probability, it->restored_probability);
            return;
        }
        restoration_options.emplace_back(-1, i, restored_probability);
    }

    /**
     * @brief Return true if the option **i** concerns arc **a**
     * @param i Option
     * @param a Arc
     * @return bool true if the option **i** concerns arc **a** false otherwise
     * @time \f$O(\#options(a))\f$
     * @space \f$O(1)\f$
     */
    bool contains(Option i, Arc a) const noexcept {
        assert(contains(i));
        auto & restoration_options = _probabilityRestorationOptions[a];
        return std::any_of(restoration_options.begin(),
                           restoration_options.end(),
                           [i](const auto & p) { return p.option == i; });
    }

    /**
     * @brief Remove the arc **a** from the option **i**
     * @param i Option
     * @param a Arc
     * @time \f$O(\#options(a))\f$
     * @space \f$O(1)\f$
     */
    void removeArc(Option i, Arc a) noexcept {
        assert(contains(i));
        auto & restoration_options = _probabilityRestorationOptions[a];
        auto it = boost::find_if(restoration_options,
                                 [i](const auto & p) { return p.option == i; });
        if(it == restoration_options.end()) return;
        restoration_options.erase(it);
    }

    /**
     * @brief Test if there is an option concerning the arc **a**
     * @param a Arc
     * @return bool true if the option **i** concerns arc **a** false otherwise
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    bool contains(Arc a) const noexcept {
        return _probabilityRestorationOptions[a].size() > 0;
    }

    /**
     * @brief Remove the arc **a** from every option
     * @param a Arc
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    void removeArc(Arc a) noexcept {
        _probabilityRestorationOptions[a].clear();
    }

    /**
     * @brief Get the number of possible actions concerning nodes
     * @return int
     * @time \f$O(|V|)\f$
     * @space \f$O(1)\f$
     */
    int getNbNodeRestorationElements() const noexcept {
        const Graph & graph = _landscape.getNetwork();
        int count = 0;
        for(typename Graph::NodeIt u(graph); u != lemon::INVALID; ++u)
            count += _qualityRestorationOptions[u].size();
        return count;
    }

    /**
     * @brief Get the number of possible actions concerning nodes
     * @return int
     * @time \f$O(|A|)\f$
     * @space \f$O(1)\f$
     */
    int getNbArcRestorationElements() const noexcept {
        const Graph & graph = _landscape.getNetwork();
        int count = 0;
        for(typename Graph::ArcIt a(graph); a != lemon::INVALID; ++a)
            count += _probabilityRestorationOptions[a].size();
        return count;
    }

    /**
     * @brief Get the number of nodes and arcs concerned by at least one option
     * @return int
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    int getNbElements() const noexcept {
        return getNbNodeRestorationElements() + getNbArcRestorationElements();
    }

    /**
     * @brief Initializes nodes restoration elements ids.
     * @time \f$O(|V| * \#options)\f$
     * @space \f$O(1)\f$
     */
    void initNodeElementIDs() {
        const Graph & graph = _landscape.getNetwork();
        int count = 0;
        for(typename Graph::NodeIt u(graph); u != lemon::INVALID; ++u)
            for(auto & restoration_element : _qualityRestorationOptions[u])
                restoration_element.id = count++;
    }

    /**
     * @brief Initializes arcs restoration elements ids.
     * @time \f$O(|A| * \#options)\f$
     * @space \f$O(1)\f$
     */
    void initArcElementIDs() {
        const Graph & graph = _landscape.getNetwork();
        int count = 0;
        for(typename Graph::ArcIt a(graph); a != lemon::INVALID; ++a)
            for(auto & restoration_element : _probabilityRestorationOptions[a])
                restoration_element.id = count++;
    }

    /**
     * @brief Initializes restoration elements ids.
     * @time \f$O((|V| + |A|) * \#options)\f$
     * @space \f$O(1)\f$
     */
    void initElementIDs() {
        initNodeElementIDs();
        initArcElementIDs();
    }

    /**
     * @brief
     * @return NodeOptionsMap
     * @time \f$O(|V| * \#options)\f$
     * @space \f$O(|V| * \#options)\f$
     */
    NodeOptionsMap computeNodeOptionsMap() const {
        NodeOptionsMap nodeOptionsMap(getNbOptions());
        for(typename Graph::NodeIt v(_landscape.getNetwork());
            v != lemon::INVALID; ++v)
            for(auto const & e : _qualityRestorationOptions[v])
                nodeOptionsMap[e.option].emplace_back(v, e.quality_gain);
        return nodeOptionsMap;
    }

    /**
     * @brief
     * @return ArcOptionsMap
     * @time \f$O(|A| * \#options)\f$
     * @space \f$O(|A| * \#options)\f$
     */
    ArcOptionsMap computeArcOptionsMap() const {
        ArcOptionsMap arcOptionsMap(getNbOptions());
        for(typename Graph::ArcIt a(_landscape.getNetwork());
            a != lemon::INVALID; ++a)
            for(auto const & e : _probabilityRestorationOptions[a])
                arcOptionsMap[e.option].emplace_back(a, e.restored_probability);
        return arcOptionsMap;
    }

    /**
     * @brief Returns the restoration options concerning the node **u**
     * @param u Node
     * @return NodeRestorationsList
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    const NodeRestorationsList & operator[](Node u) const noexcept {
        return _qualityRestorationOptions[u];
    }

    /**
     * @brief Returns the restoration options concerning the arc **a**
     * @param a Arc
     * @return ArcRestorationsList
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    const ArcRestorationsList & operator[](Arc a) const noexcept {
        return _probabilityRestorationOptions[a];
    }

    /**
     * @brief Returns the restoration options concerning the node **u**
     * @param u Node
     * @return NodeRestorationsList
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    NodeRestorationsList & operator[](Node u) noexcept {
        return _qualityRestorationOptions[u];
    }

    /**
     * @brief Returns the restoration options concerning the arc **a**
     * @param a Arc
     * @return ArcRestorationsList
     * @time \f$O(1)\f$
     * @space \f$O(1)\f$
     */
    ArcRestorationsList & operator[](Arc a) noexcept {
        return _probabilityRestorationOptions[a];
    }

    /**
     * @brief Computes the total cost of the restoration plan, i.e. sum of
     * options costs
     * @return double
     */
    double totalCost() const noexcept {
        return std::accumulate(_costs.begin(), _costs.end(), 0.0);
    }

    /**
     * @brief Return a view for iterating over the option objects
     * @return auto
     */
    auto options() const {
        // auto view = ranges::views::transform(
        //     ranges::iota_view<int, int>(0, static_cast<int>(getNbOptions())),
        //     [](int constr_id) { return Option(constr_id); });
        auto view = ranges::iota_view<int, int>(0, static_cast<int>(getNbOptions()));
        return view;
    }
};

template <typename LS>  // requires concepts::IsLandscape<LS> //c++20
std::ostream & operator<<(std::ostream & in, const RestorationPlan<LS> & plan);

#endif  // RESTORATION_PLAN_HPP