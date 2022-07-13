/**
 * @file instances_helper.hpp
 * @author francois.hamonic@gmail.com
 * @brief parse specific instances datas
 * @version 0.1
 * @date 2020-05-08
 */
#ifndef INSTANCES_HELPER_HPP
#define INSTANCES_HELPER_HPP

#include <math.h>
#include <algorithm>
#include <execution>
#include <numeric>
#include <random>

#include <lemon/adaptors.h>

#include <boost/range/algorithm/sort.hpp>

#include "landscape/mutable_landscape.hpp"
#include "solvers/concept/instance.hpp"

#include "fast-cpp-csv-parser/csv.h"
#include "utils/random_chooser.hpp"

void addCostNoise(Instance & instance, double deviation_ratio = 0.2,
                  int seed = 456) {
    std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution(1.0, deviation_ratio);

    auto noise = [&generator, &distribution](double value) {
        return std::max(std::numeric_limits<double>::epsilon(),
                        value * distribution(generator));
    };

    for(const RestorationPlan<MutableLandscape>::Option i :
        instance.plan.options())
        instance.plan.setCost(i, noise(instance.plan.getCost(i)));
}

Instance make_instance_aude(const double median,
                            const double fish_ladder_prob) {
    Instance instance;

    MutableLandscape & landscape = instance.landscape;
    const MutableLandscape::Graph & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape> & plan = instance.plan;

    std::array<MutableLandscape::Node, 45> nodes;
    MutableLandscape::Graph::NodeMap<double> troncons_lengths(graph);
    // MutableLandscape::Graph::NodeMap<int> depth_id(graph);

    auto p = [median](const double d) {
        return std::exp(d / median * std::log(0.5));
    };

    io::CSVReader<4> patches("../landscape_opt_datas/Aude/aude.patches");
    patches.read_header(io::ignore_extra_column, "id", "length", "x", "y");
    int id;
    double length, X, Y;
    while(patches.read_row(id, length, X, Y)) {
        MutableLandscape::Node u = landscape.addNode(length, Point(X, Y));
        nodes[id] = u;
        troncons_lengths[u] = length;
    }

    io::CSVReader<3> links("../landscape_opt_datas/Aude/aude.links");
    links.read_header(io::ignore_extra_column, "source_id", "target_id", "dam");
    int source_id, target_id, dam;
    while(links.read_row(source_id, target_id, dam)) {
        if(source_id > target_id) continue;

        MutableLandscape::Node u = nodes[source_id];
        MutableLandscape::Node v = nodes[target_id];

        const double prob = p((troncons_lengths[u] + troncons_lengths[v]) / 2);

        if(!dam) {
            landscape.addArc(nodes[source_id], nodes[target_id], prob);
            landscape.addArc(nodes[target_id], nodes[source_id], prob);
            continue;
        }
        MutableLandscape::Arc a1 =
            landscape.addArc(nodes[source_id], nodes[target_id], 0);
        MutableLandscape::Arc a2 =
            landscape.addArc(nodes[target_id], nodes[source_id], 0);
        RestorationPlan<MutableLandscape>::Option option = plan.addOption(1);
        plan.addArc(option, a1, fish_ladder_prob * prob);
        plan.addArc(option, a2, fish_ladder_prob * prob);
    }

    return instance;
}

Instance make_instance_quebec_leam(double pow, double thresold, double median,
                                   double decreased_prob,
                                   Point orig = Point(240548, 4986893),
                                   Point dim = Point(32360, 20000)) {
    Instance instance;

    MutableLandscape & landscape = instance.landscape;
    const MutableLandscape::Graph & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape> & plan = instance.plan;

    auto p = [median, pow](const double d) {
        return std::exp(std::pow(d, pow) / std::pow(median, pow) *
                        std::log(0.5));
    };

    std::array<MutableLandscape::Node, 8248> node_correspondance;
    node_correspondance.fill(lemon::INVALID);

    using ThreatData = struct {
        MutableLandscape::Node node;
        double area;
    };
    std::vector<ThreatData> threaten_list;

    io::CSVReader<5> patches(
        "../landscape_opt_datas/quebec_leam_v3/raw/sommets_leam_v3.csv");
    patches.read_header(io::ignore_extra_column, "count", "area", "xcoord",
                        "ycoord", "count2050");
    int id;
    double area, xcoord, ycoord, count2050;
    while(patches.read_row(id, area, xcoord, ycoord, count2050)) {
        if(xcoord < orig.x) continue;
        if(xcoord >= orig.x + dim.x) continue;
        if(ycoord < orig.y) continue;
        if(ycoord >= orig.y + dim.y) continue;

        MutableLandscape::Node u =
            landscape.addNode(count2050, Point(xcoord, ycoord));
        node_correspondance[id] = u;

        if(area == count2050) continue;
        if(area > 0 && count2050 == 0) {
            threaten_list.push_back(ThreatData{u, area});
            continue;
        }

        const double area_loss = area - count2050;
        RestorationPlan<MutableLandscape>::Option option =
            plan.addOption(area_loss);
        plan.addNode(option, u, area_loss);
    }

    io::CSVReader<3> links(
        "../landscape_opt_datas/quebec_leam_v3/raw/aretes_leam_v3.csv");
    links.read_header(io::ignore_extra_column, "from", "to", "Dist");
    int from, to;
    double Dist;
    while(links.read_row(from, to, Dist)) {
        MutableLandscape::Node u = node_correspondance[from];
        MutableLandscape::Node v = node_correspondance[to];
        if(u == lemon::INVALID || v == lemon::INVALID) continue;
        double probability = p(Dist);
        if(probability < thresold) continue;
        landscape.addArc(u, v, probability);
        landscape.addArc(v, u, probability);
    }

    for(ThreatData data : threaten_list) {
        MutableLandscape::Node v1 = data.node;
        RestorationPlan<MutableLandscape>::Option option =
            plan.addOption(data.area);
        landscape.setCoords(v1, landscape.getCoords(v1) + Point(-200, 0));
        MutableLandscape::Node v2 =
            landscape.addNode(0, landscape.getCoords(v1) + Point(200, 0));

        for(MutableLandscape::Graph::OutArcIt a(graph, v1), next_a = a;
            a != lemon::INVALID; a = next_a) {
            ++next_a;
            landscape.changeSource(a, v2);
        }
        MutableLandscape::Arc v1v2 = landscape.addArc(v1, v2, decreased_prob);
        plan.addArc(option, v1v2, 1);
        plan.addNode(option, v2, data.area);
    }

    return instance;
}

Instance make_instance_quebec_frog(double pow, double thresold, double median) {
    Instance instance;

    MutableLandscape & landscape = instance.landscape;
    const MutableLandscape::Graph & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape> & plan = instance.plan;

    auto p = [median, pow](const double d) {
        return std::exp(std::pow(d, pow) / std::pow(median, pow) *
                        std::log(0.5));
    };

    std::array<MutableLandscape::Node, 3032> node_correspondance;
    node_correspondance.fill(lemon::INVALID);

    using ThreatData = struct {
        MutableLandscape::Node node;
        double area;
    };
    std::vector<ThreatData> threaten_list;

    io::CSVReader<5> patches(
        "../landscape_opt_datas/quebec_438_RASY/vertices_438_RASY.csv");
    patches.read_header(io::ignore_extra_column, "name", "area", "xcoord",
                        "ycoord", "per_menace");
    int id;
    double area_in_2000, xcoord, ycoord, per_menace;
    while(patches.read_row(id, area_in_2000, xcoord, ycoord, per_menace)) {
        area_in_2000 /= 100;
        const double area_loss_by_2050 = area_in_2000 * per_menace;
        const double area_in_2050 = area_in_2000 - area_loss_by_2050;

        MutableLandscape::Node u =
            landscape.addNode(area_in_2050, Point(xcoord, ycoord));
        node_correspondance[id] = u;

        if(per_menace == 0) continue;
        if(per_menace == 1) {
            threaten_list.push_back(ThreatData{u, area_in_2000});
            continue;
        }

        RestorationPlan<MutableLandscape>::Option option =
            plan.addOption(area_loss_by_2050);
        plan.addNode(option, u, area_loss_by_2050);
    }

    io::CSVReader<3> links(
        "../landscape_opt_datas/quebec_438_RASY/edges_438_RASY.csv");
    links.read_header(io::ignore_extra_column, "from", "to", "Dist");
    int from, to;
    double Dist;
    while(links.read_row(from, to, Dist)) {
        MutableLandscape::Node u = node_correspondance[from];
        MutableLandscape::Node v = node_correspondance[to];
        if(u == lemon::INVALID || v == lemon::INVALID) continue;
        double probability = p(Dist);
        if(probability < thresold) continue;
        landscape.addArc(u, v, probability);
        landscape.addArc(v, u, probability);
    }

    for(ThreatData data : threaten_list) {
        MutableLandscape::Node v1 = data.node;
        RestorationPlan<MutableLandscape>::Option option =
            plan.addOption(data.area);
        landscape.setCoords(v1, landscape.getCoords(v1) + Point(-200, 0));
        MutableLandscape::Node v2 =
            landscape.addNode(0, landscape.getCoords(v1) + Point(200, 0));

        for(MutableLandscape::Graph::OutArcIt a(graph, v1), next_a = a;
            a != lemon::INVALID; a = next_a) {
            ++next_a;
            landscape.changeSource(a, v2);
        }
        MutableLandscape::Arc v1v2 = landscape.addArc(v1, v2, 0);
        plan.addArc(option, v1v2, 1);
        plan.addNode(option, v1, data.area);
    }

    return instance;
}

Instance make_instance_biorevaix_level_2_v7(const double restoration_coef = 2,
                                            const double distance_coef = 1) {
    Instance instance;
    MutableLandscape & landscape = instance.landscape;
    const MutableLandscape::Graph & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape> & plan = instance.plan;

    auto prob = [distance_coef](const double cost) {
        return cost == 1     ? std::pow(1, distance_coef)
               : cost == 10  ? std::pow(0.98, distance_coef)
               : cost == 150 ? std::pow(0.8, distance_coef)
               : cost == 300 ? std::pow(0.6, distance_coef)
               : cost == 800 ? std::pow(0.4, distance_coef)
                             : 0;
    };

    std::array<MutableLandscape::Node, 688402> nodes;
    nodes.fill(lemon::INVALID);
    MutableLandscape::Graph::NodeMap<double> node_prob(graph, 0.0);
    MutableLandscape::Graph::NodeMap<RestorationPlan<MutableLandscape>::Option>
        troncon_option(graph);
    std::array<RestorationPlan<MutableLandscape>::Option, 2008> id_tronc_option;
    id_tronc_option.fill(-1);

    io::CSVReader<7> patches(
        "../landscape_opt_datas/BiorevAix/vertexN2_v7.txt");
    patches.read_header(io::ignore_extra_column, "N2_id", "X", "Y", "N4_id",
                        "area2", "area4", "cost_mode");
    int N2_id, N4_id;
    int area2, area4;
    double X, Y, cost;
    while(patches.read_row(N2_id, X, Y, N4_id, area2, area4, cost)) {
        if(!area2) continue;
        // if(!area4) continue;
        if(cost == 1000) continue;
        MutableLandscape::Node u =
            landscape.addNode((cost == 1 ? 1 : 0), Point(X, Y));
        nodes[N2_id] = u;
        node_prob[u] = prob(cost);
        troncon_option[u] = -1;

        if(cost != 800) continue;
        RestorationPlan<MutableLandscape>::Option & option =
            id_tronc_option[N4_id];
        if(option == -1) option = plan.addOption(0);
        troncon_option[u] = option;
        plan.setCost(option, plan.getCost(option) + 1);
    }

    io::CSVReader<2> links("../landscape_opt_datas/BiorevAix/AL_N2.txt");
    links.read_header(io::ignore_extra_column, "from", "to");
    int from, to;
    while(links.read_row(from, to)) {
        const MutableLandscape::Node u = nodes[from];
        const MutableLandscape::Node v = nodes[to];
        if(u == lemon::INVALID || v == lemon::INVALID) continue;
        if(node_prob[u] == 0 || node_prob[v] == 0) continue;

        RestorationPlan<MutableLandscape>::Option option_u = troncon_option[u];
        RestorationPlan<MutableLandscape>::Option option_v = troncon_option[v];
        if(option_u > 0 && option_v > 0 && option_u != option_v) {
            const MutableLandscape::Node w = landscape.addNode(
                0, (landscape.getCoords(u) + landscape.getCoords(v)) / 2);

            const MutableLandscape::Arc uw =
                landscape.addArc(u, w, std::sqrt(node_prob[u]));
            const MutableLandscape::Arc wu =
                landscape.addArc(w, u, std::sqrt(node_prob[u]));
            const MutableLandscape::Arc wv =
                landscape.addArc(w, v, std::sqrt(node_prob[v]));
            const MutableLandscape::Arc vw =
                landscape.addArc(v, w, std::sqrt(node_prob[v]));

            const double restored_prob_u =
                std::pow(node_prob[u], 1 / (2 * restoration_coef));
            const double restored_prob_v =
                std::pow(node_prob[v], 1 / (2 * restoration_coef));

            plan.addArc(option_u, uw, restored_prob_u);
            plan.addArc(option_u, wu, restored_prob_u);
            plan.addArc(option_v, vw, restored_prob_v);
            plan.addArc(option_v, wv, restored_prob_v);
            continue;
        };

        double probability = std::sqrt(node_prob[u] * node_prob[v]);
        probability = std::max(std::min(probability, 1.0), 0.0);

        const MutableLandscape::Arc uv = landscape.addArc(u, v, probability);
        const MutableLandscape::Arc vu = landscape.addArc(v, u, probability);

        if(option_u < 0 && option_v < 0) continue;

        RestorationPlan<MutableLandscape>::Option option =
            std::max(option_u, option_v);
        double restored_prob =
            std::pow(node_prob[u],
                     1 / (2 * (option_u >= 0 ? restoration_coef : 1))) *
            std::pow(node_prob[v],
                     1 / (2 * (option_v >= 0 ? restoration_coef : 1)));
        if(restored_prob <= probability) continue;
        plan.addArc(option, uv, restored_prob);
        plan.addArc(option, vu, restored_prob);
    }
    return instance;
}

Instance make_instance_biorevaix_level_2_all_troncons(
    const double restoration_coef = 2, const double distance_coef = 1) {
    Instance instance;
    MutableLandscape & landscape = instance.landscape;
    const MutableLandscape::Graph & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape> & plan = instance.plan;

    auto prob = [distance_coef](const double cost) {
        return cost == 1     ? std::pow(1, distance_coef)
               : cost == 10  ? std::pow(0.98, distance_coef)
               : cost == 150 ? std::pow(0.8, distance_coef)
               : cost == 300 ? std::pow(0.6, distance_coef)
               : cost == 800 ? std::pow(0.4, distance_coef)
                             : 0;
    };

    std::array<MutableLandscape::Node, 98344> nodes;
    nodes.fill(lemon::INVALID);
    MutableLandscape::Graph::NodeMap<double> node_prob(graph, 0.0);

    io::CSVReader<6> patches(
        "../landscape_opt_datas/BiorevAix/vertexN2_v7.txt");
    patches.read_header(io::ignore_extra_column, "N2_id", "X", "Y", "area2",
                        "area4", "cost_mode");
    int N2_id;
    int area2, area4;
    double X, Y, cost;
    while(patches.read_row(N2_id, X, Y, area2, area4, cost)) {
        // if(!area2) continue;
        // if(!area4) continue;
        if(cost == 1000) continue;
        MutableLandscape::Node u =
            landscape.addNode((cost == 1 ? 1 : 0), Point(X, Y));
        nodes[N2_id] = u;
        node_prob[u] = prob(cost);
    }

    io::CSVReader<2> links("../landscape_opt_datas/BiorevAix/AL_N2.txt");
    links.read_header(io::ignore_extra_column, "from", "to");
    int from, to;
    while(links.read_row(from, to)) {
        const MutableLandscape::Node u = nodes[from];
        const MutableLandscape::Node v = nodes[to];
        if(u == lemon::INVALID || v == lemon::INVALID) continue;
        if(node_prob[u] == 0 || node_prob[v] == 0) continue;

        double probability = std::sqrt(node_prob[u] * node_prob[v]);
        probability = std::max(std::min(probability, 1.0), 0.0);

        const MutableLandscape::Arc uv = landscape.addArc(u, v, probability);
        const MutableLandscape::Arc vu = landscape.addArc(v, u, probability);
    }

    std::array<RestorationPlan<MutableLandscape>::Option, 5460> troncon_option;
    for(int i = 0; i < 5460; ++i) {
        troncon_option[i] = plan.addOption(0);
    }
    io::CSVReader<2> troncons(
        "../landscape_opt_datas/BiorevAix/croisemt_troncon_hexagN2.txt");
    troncons.read_header(io::ignore_extra_column, "troncon_id", "N2_id");
    int troncon_id;
    while(troncons.read_row(troncon_id, N2_id)) {
        const MutableLandscape::Node u = nodes[N2_id];

        RestorationPlan<MutableLandscape>::Option option =
            troncon_option[troncon_id];

        plan.setCost(option, plan.getCost(option) + 1);

        for(MutableLandscape::Graph::OutArcIt uv(graph, u);
            uv != lemon::INVALID; ++uv) {
            const MutableLandscape::Node v = graph.target(uv);
            const double probability = landscape.getProbability(uv);
            double restored_prob =
                std::pow(node_prob[u], 1 / (2 * restoration_coef)) *
                std::pow(node_prob[v], 1 / 2);
            if(restored_prob <= probability) continue;
            plan.addArc(option, uv, restored_prob);
        }
    }
    return instance;
}

Instance make_instance_biorevaix_level_1_all_troncons_updated(
    const double restoration_coef = 2, const double distance_coef = 1) {
    Instance instance;
    MutableLandscape & landscape = instance.landscape;
    const MutableLandscape::Graph & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape> & plan = instance.plan;

    const double light_vege_prob = std::pow(0.8, distance_coef);
    const double road_prob = std::pow(0.4, distance_coef);
    const double vege_road_prob = std::pow(0.5, distance_coef);

    auto prob = [&](const double cost) {
        return cost == 1     ? std::pow(1, distance_coef)
               : cost == 10  ? std::pow(0.98, distance_coef)
               : cost == 150 ? light_vege_prob
               : cost == 300 ? std::pow(0.6, distance_coef)
               : cost == 800 ? road_prob
                             : 0;
    };

    std::array<MutableLandscape::Node, 688408> nodes1;
    nodes1.fill(lemon::INVALID);
    std::array<std::vector<MutableLandscape::Node>, 98344> nodes2;
    MutableLandscape::Graph::NodeMap<double> node_prob(graph, 0.0);
    MutableLandscape::Graph::NodeMap<double> enhanced_node_prob(graph, 0.0);

    io::CSVReader<5> patches(
        "../landscape_opt_datas/BiorevAix/raw/vertexN1_v2.csv");
    patches.read_header(io::ignore_extra_column, "N1_id", "X", "Y", "N2_id",
                        "cost");
    int N1_id, N2_id;
    double X, Y, cost;
    while(patches.read_row(N1_id, X, Y, N2_id, cost)) {
        if(cost == 1000) continue;
        MutableLandscape::Node u =
            landscape.addNode((cost == 1 ? 1 : 0), Point(X, Y));
        nodes1[N1_id] = u;
        nodes2[N2_id].push_back(u);
        node_prob[u] = prob(cost);
    }

    io::CSVReader<2> links("../landscape_opt_datas/BiorevAix/raw/AL_N1.csv");
    links.read_header(io::ignore_extra_column, "from", "to");
    int from, to;
    while(links.read_row(from, to)) {
        const MutableLandscape::Node u = nodes1[from];
        const MutableLandscape::Node v = nodes1[to];
        if(u == lemon::INVALID || v == lemon::INVALID) continue;
        if(node_prob[u] == 0 || node_prob[v] == 0) continue;

        double probability = std::sqrt(node_prob[u] * node_prob[v]);
        probability = std::max(std::min(probability, 1.0), 0.0);

        const MutableLandscape::Arc uv = landscape.addArc(u, v, probability);
        const MutableLandscape::Arc vu = landscape.addArc(v, u, probability);
    }

    std::array<RestorationPlan<MutableLandscape>::Option, 5460> troncon_option;
    for(int i = 0; i < 5460; ++i) {
        troncon_option[i] = plan.addOption(0);
    }
    io::CSVReader<2> troncons(
        "../landscape_opt_datas/BiorevAix/croisemt_troncon_hexagN2.txt");
    troncons.read_header(io::ignore_extra_column, "troncon_id", "N2_id");
    int troncon_id;

    for(MutableLandscape::Graph::NodeIt u(graph, u); u != lemon::INVALID; ++u)
        enhanced_node_prob[u] = node_prob[u];

    while(troncons.read_row(troncon_id, N2_id)) {
        for(const MutableLandscape::Node & u : nodes2[N2_id]) {
            if(enhanced_node_prob[u] == 0) continue;
            if(enhanced_node_prob[u] == road_prob) enhanced_node_prob[u] = vege_road_prob;
            enhanced_node_prob[u] = light_vege_prob;
        }

        for(const MutableLandscape::Node & u : nodes2[N2_id]) {
            RestorationPlan<MutableLandscape>::Option option =
                troncon_option[troncon_id];

            plan.setCost(option, plan.getCost(option) + 1);

            for(MutableLandscape::Graph::OutArcIt uv(graph, u);
                uv != lemon::INVALID; ++uv) {
                const MutableLandscape::Node v = graph.target(uv);
                const double probability = landscape.getProbability(uv);
                double restored_prob = std::sqrt(enhanced_node_prob[u] * enhanced_node_prob[v]);
                if(restored_prob <= probability) continue;
                plan.addArc(option, uv, restored_prob);
            }
        }

        for(const MutableLandscape::Node & u : nodes2[N2_id]) {
            enhanced_node_prob[u] = node_prob[u];
        }
    }
    return instance;
}

Instance make_instance_marseille(double pow, double thresold, double median,
                                 int nb_friches = 100) {
    Instance instance;

    MutableLandscape & landscape = instance.landscape;
    const MutableLandscape::Graph & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape> & plan = instance.plan;

    auto d = [&landscape](MutableLandscape::Node u, MutableLandscape::Node v) {
        return std::sqrt(
            (landscape.getCoords(u) - landscape.getCoords(v)).normSquare());
    };
    auto p = [median, pow](const double d) {
        return std::exp(std::pow(d, pow) / std::pow(median, pow) *
                        std::log(0.5));
    };

    using FricheData = struct {
        Point p;
        double area;
        double price;
        MutableLandscape::Node node;
    };
    RandomChooser<FricheData> friches_chooser(9876);
    std::vector<FricheData> friches_list;

    io::CSVReader<5> patches(
        "../landscape_opt_datas/Marseille/vertices_marseillec.csv");
    patches.read_header(io::ignore_extra_column, "category", "x", "y", "area2",
                        "price_rel");
    std::string category;
    double x, y, area, price_rel;
    while(patches.read_row(category, x, y, area, price_rel)) {
        if(category.compare("\"massif\"") == 0) {
            MutableLandscape::Node u = landscape.addNode(20, Point(x, y));
            (void)u;
            ///////////////////////////////////////////////////////////////////
            // std::cout << std::setprecision(10) << graph.id(u) << ",massif,"
            //           << area << "," << x << "," << y << std::endl;
            ///////////////////////////////////////////////////////////////////
            continue;
        }
        if(category.compare("\"parc\"") == 0) {
            MutableLandscape::Node u = landscape.addNode(area, Point(x, y));
            (void)u;
            ///////////////////////////////////////////////////////////////////
            // std::cout << graph.id(u) << ",parc," << area << "," << x << ","
            // << y
            //           << std::endl;
            ///////////////////////////////////////////////////////////////////
            continue;
        }
        if(category.compare("\"friche\"") == 0) {
            friches_chooser.add(
                FricheData{Point(x, y), area, price_rel * area, lemon::INVALID},
                1);
            continue;
        }
        assert(false);
    }
    for(int i = 0; i < nb_friches; i++) {
        if(!friches_chooser.canPick()) break;
        FricheData data = friches_chooser.pick();
        MutableLandscape::Node u = landscape.addNode(0, data.p);
        ///////////////////////////////////////////////////////////////////////
        // std::cout << graph.id(u) << ",friche," << area << "," << data.p.x <<
        // ","
        //           << data.p.y << std::endl;
        ///////////////////////////////////////////////////////////////////////
        data.node = u;
        friches_list.push_back(data);
    }

    for(MutableLandscape::NodeIt u(graph); u != lemon::INVALID; ++u) {
        for(MutableLandscape::NodeIt v(graph); v != lemon::INVALID; ++v) {
            if(v < u || u == v) continue;
            double dist = d(u, v);
            double probability = p(dist);
            if(probability < thresold) continue;
            landscape.addArc(u, v, probability);
            landscape.addArc(v, u, probability);
            ///////////////////////////////////////////////////////////////////////
            std::cout << graph.id(u) << "," << graph.id(v) << ","
                      << landscape.getCoords(u).x << ","
                      << landscape.getCoords(u).y << ","
                      << landscape.getCoords(v).x << ","
                      << landscape.getCoords(v).y << std::endl;
            ///////////////////////////////////////////////////////////////////////
        }
    }

    for(FricheData data : friches_list) {
        MutableLandscape::Node v1 = data.node;
        RestorationPlan<MutableLandscape>::Option option =
            plan.addOption(data.price);
        MutableLandscape::Node v2 = landscape.addNode(
            0, landscape.getCoords(v1) + Point(0.0001, 0.0001));

        for(MutableLandscape::Graph::OutArcIt a(graph, v1), next_a = a;
            a != lemon::INVALID; a = next_a) {
            ++next_a;
            landscape.changeSource(a, v2);
        }
        MutableLandscape::Arc v1v2 = landscape.addArc(v1, v2, 0);
        plan.addArc(option, v1v2, 1);
        plan.addNode(option, v2, data.area);
    }

    return instance;
}

#endif  // INSTANCES_HELPER_HPP