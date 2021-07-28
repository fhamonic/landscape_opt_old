#ifndef TRIVIAL_REFORMULATION_HPP
#define TRIVIAL_REFORMULATION_HPP

#include "landscape/mutable_landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"
#include "solvers/concept/instance.hpp"

#include <lemon/connectivity.h>
#include <range/v3/view/subrange.hpp>
#include <range/v3/view/zip.hpp>

/**
 * Erase every arc whose probability in the best scenario is negligeable, 
 * according to **epsilon**.
 * 
 * @time \f$O(|A|)\f$
 * @space \f$O(1)\f$
 */
void remove_zero_probability_arcs(MutableLandscape & landscape, 
                    const RestorationPlan<MutableLandscape> & plan, 
                    const double epsilon=1.0e-13);

/**
 * Erase every node that cannot carry flow because its quality is negligeable, 
 * according to **epsilon**, in the best scenario, it cannot be enhanced and 
 * there is no path from a positive quality node to it.
 * 
 * @time \f$O(|V| + |A|)\f$
 * @space \f$O(|V|)\f$
 */
void remove_no_flow_nodes(MutableLandscape & landscape, 
                    const RestorationPlan<MutableLandscape> & plan,
                    const double epsilon=1.0e-13);

/**
 * Contract the strongly connected components of the subgraph 
 * induced by the arcs of probabilty 1.
 * 
 * @time \f$O(|V| + |A|)\f$
 * @space \f$O(|A|)\f$
 */
void contract_patches_components(MutableLandscape & landscape, RestorationPlan<MutableLandscape> & plan);

/**
 * Aggregate parallel arcs.
 * 
 * @time \f$O((|V| + |A|) * \#options)\f$
 * @space \f$O(|V|)\f$
 */
void aggregate_parallel_arcs(MutableLandscape & landscape, RestorationPlan<MutableLandscape> & plan);

/**
 * Copy the instance passed in parameter and, doing so, compact it by removing 
 * invalid nodes and arcs from **landscape** and empty options from **plan**.
 * This invalidates all IDs referring to the copied nodes, arcs and options.
 * 
 * @time \f$O((|V| + |A|) * \#options)\f$
 * @space \f$O((|V| + |A|) * \#options)\f$
 */
Instance copy_and_compact_instance(const MutableLandscape & landscape, 
                  const RestorationPlan<MutableLandscape> & plan);
Instance copy_and_compact_instance(const Instance & instance);

/**
 * Performs trivial simplifications of the given instance such as : 
 * - removing arc whose probability is negligeable 
 * - removing nodes that can only carry negligeable flow 
 * - contracting the strongly connected components formed by 
 *      arcs of probability 1 
 * - aggregating parallels arcs 
 * 
 * @time \f$O((|V| + |A|) * \#options)\f$
 * @space \f$O((|V| + |A|) * \#options)\f$
 */
Instance trivial_reformulate(MutableLandscape&& landscape, 
            RestorationPlan<MutableLandscape>&& plan);
Instance trivial_reformulate(Instance&& instance);
Instance trivial_reformulate(const MutableLandscape & landscape,
            const RestorationPlan<MutableLandscape> & plan);
Instance trivial_reformulate(const Instance & instance);


#endif //TRIVIAL_REFORMULATION_HPP