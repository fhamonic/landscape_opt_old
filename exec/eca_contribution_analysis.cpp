#include <filesystem>
#include <fstream>
#include <iostream>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include "algorithms/multiplicative_dijkstra.hpp"

#include "parsers/std_mutable_landscape_parser.hpp"
#include "parsers/std_restoration_plan_parser.hpp"

#include "helper.hpp"
#include "instances_helper.hpp"

#include "precomputation/trivial_reformulation.hpp"

using Accumulator = boost::accumulators::accumulator_set<
    double, boost::accumulators::features<boost::accumulators::tag::mean,
                                          boost::accumulators::tag::median>>;

int main() {
    std::ofstream data_log("output/data.log");
    data_log << std::fixed << std::setprecision(6);
    data_log << "ball_diameter "
             << "average_percentage_of_nodes "
             << "median_percentage_of_nodes "
             << "average_percentage_of_contribution "
             << "median_percentage_of_contribution " << std::endl;

    constexpr int nb_partitions = 40;
    std::array<Accumulator, nb_partitions> percent_node_accs;
    std::array<Accumulator, nb_partitions> percent_contribution_accs;

    auto prob_to_partition = [&](double prob) {
        return static_cast<int>((nb_partitions) * (1 - prob));
    };

    using Graph = MutableLandscape::Graph;
    using Node = MutableLandscape::Node;
    using NodeIt = MutableLandscape::NodeIt;

    Instance raw_instance = make_instance_biorevaix_level_2_v7(6);
    Instance instance = trivial_reformulate(std::move(raw_instance));
    const MutableLandscape & landscape = instance.landscape;

    const Graph & graph = landscape.getNetwork();

    const int node_count = lemon::countNodes(graph);

    for(NodeIt s(graph); s != lemon::INVALID; ++s) {
        int last_partition = 0;
        int node_count_sum = 0;
        double contribution_sum = 0;

        auto sorted_pairs = Helper::computeDistancePairs(landscape, s);
        const double contribution_max = std::transform_reduce(
            sorted_pairs.begin(), sorted_pairs.end(), 0.0, std::plus<double>(),
            [&](const auto & p) {
                return landscape.getQuality(s) * landscape.getQuality(p.first) *
                       p.second;
            });

        for(const auto & [t, p_st] : sorted_pairs) {
            if(p_st == 0) break;
            const int current_partition = prob_to_partition(p_st);
            if(current_partition > last_partition) {
                percent_node_accs[last_partition](static_cast<double>(node_count_sum) / node_count * 100);
                percent_contribution_accs[last_partition](contribution_sum / contribution_max * 100);

                last_partition = current_partition;
            }

            ++node_count_sum;
            contribution_sum +=
                landscape.getQuality(s) * landscape.getQuality(t) * p_st;
        }
    }

    for(int i = 0; i < nb_partitions; ++i) {
        const double diameter = 1 - (static_cast<double>(i) / nb_partitions);
        data_log << diameter << " "
                 << boost::accumulators::mean(percent_node_accs[i]) << " "
                 << boost::accumulators::median(percent_node_accs[i]) << " "
                 << boost::accumulators::mean(percent_contribution_accs[i])
                 << " "
                 << boost::accumulators::median(percent_contribution_accs[i])
                 << std::endl;
    }

    return EXIT_SUCCESS;
}