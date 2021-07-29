/**
 * @file instances_helper.hpp
 * @author francois.hamonic@gmail.com
 * @brief parse specific instances datas
 * @version 0.1
 * @date 2020-05-08 
 */
#ifndef INSTANCES_HELPER_HPP
#define INSTANCES_HELPER_HPP

#include <algorithm>
#include <execution>
#include <math.h>
#include <numeric>
#include <random>

#include <lemon/adaptors.h>

#include <boost/range/algorithm/sort.hpp>

#include "landscape/mutable_landscape.hpp"
#include "solvers/concept/instance.hpp"

#include "fast-cpp-csv-parser/csv.h"
#include "utils/random_chooser.hpp"

void addCostNoise(Instance & instance, double deviation_ratio=0.2, int seed=456) {
    std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution(1.0, deviation_ratio);

    auto noise = [&generator, &distribution] (double value) {
        return std::max(std::numeric_limits<double>::epsilon(), value * distribution(generator));
    };

    for(RestorationPlan<MutableLandscape>::Option i=0; i<instance.plan.getNbOptions(); ++i)
        instance.plan.setCost(i, noise(instance.plan.getCost(i)));
}


Instance make_instance_marseillec(double pow, double thresold, double median, int nb_friches=100) {
    Instance instance;
    
    MutableLandscape & landscape = instance.landscape;
    const Graph_t & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape> & plan = instance.plan;

    auto d = [&landscape] (Graph_t::Node u, Graph_t::Node v) { return std::sqrt((landscape.getCoords(u) - landscape.getCoords(v)).normSquare()); };
    auto p = [median, pow] (const double d) { return std::exp(std::pow(d,pow)/std::pow(median, pow)*std::log(0.5)); };

    using FricheData = struct { Point p; double area; double price; Graph_t::Node node; };
    RandomChooser<FricheData> friches_chooser(9876);
    std::vector<FricheData> friches_list;

    io::CSVReader<5> patches("data/Marseille/vertices_marseillec.csv");
    patches.read_header(io::ignore_extra_column,"category", "x", "y", "area2", "price_rel");
    std::string category;
    double x, y, area, price_rel;
    while(patches.read_row(category, x, y, area, price_rel)) {
        if(category.compare("\"massif\"") == 0) { landscape.addNode(20, Point(x,y)); continue; }
        if(category.compare("\"parc\"") == 0) { landscape.addNode(area, Point(x,y)); continue; }
        if(category.compare("\"friche\"") == 0) { friches_chooser.add(FricheData{Point(x,y), area, price_rel * area, lemon::INVALID}, 1); continue; }
        assert(false);
    }
    for(int i=0; i<nb_friches; i++) {
        if(!friches_chooser.canPick()) break;
        FricheData data = friches_chooser.pick();
        Graph_t::Node u = landscape.addNode(0, data.p);
        data.node = u;
        friches_list.push_back(data);
    }

    for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) {
        for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
            if(v < u || u == v) continue;
            double dist = d(u,v);
            double probability = p(dist);
            if(probability < thresold) continue;
            landscape.addArc(u, v, probability);
            landscape.addArc(v, u, probability);
        }
    }

    for(FricheData data : friches_list) {
        Graph_t::Node v1 = data.node;
        RestorationPlan<MutableLandscape>::Option option = plan.addOption(data.price);
        Graph_t::Node v2 = landscape.addNode(0, landscape.getCoords(v1) + Point(0.0001, 0.0001));
        
        for(Graph_t::OutArcIt a(graph, v1), next_a = a; a != lemon::INVALID; a = next_a) {
            ++next_a;
            landscape.changeSource(a, v2);
        }
        Graph_t::Arc v1v2 = landscape.addArc(v1, v2, 0);
        plan.addArc(option, v1v2, 1);
        plan.addNode(option, v2, data.area);
    }

    return instance;
}


Instance make_instance_quebec(double pow, double thresold, double median,
        Point orig=Point(240548, 4986893), Point dim=Point(32360, 20000)) {
    Instance instance;
    
    MutableLandscape & landscape = instance.landscape;
    const Graph_t & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape>& plan = instance.plan;

    auto p = [median, pow] (const double d) { return std::exp(std::pow(d,pow)/std::pow(median, pow)*std::log(0.5)); };
    
    std::vector<Graph_t::Node> node_correspondance;
    using ThreatData = struct { Graph_t::Node node; double area; };
    std::vector<ThreatData> threaten_list;

    io::CSVReader<4> patches("data/quebec_leam_v3/raw/sommets_leam_v3.csv");
    patches.read_header(io::ignore_extra_column, "area", "xcoord", "ycoord", "count2050");
    double area, xcoord, ycoord, count2050;
    while(patches.read_row(area, xcoord, ycoord, count2050)) {
        node_correspondance.push_back(lemon::INVALID);
        if(xcoord < orig.x) continue;
        if(xcoord >= orig.x + dim.x) continue;
        if(ycoord < orig.y) continue;
        if(ycoord >= orig.y + dim.y) continue;
        
        Graph_t::Node u = landscape.addNode(count2050, Point(xcoord,ycoord));
        node_correspondance[node_correspondance.size()-1] = u;

        if(area == count2050) continue;
        if(area > 0 && count2050 == 0) {
            threaten_list.push_back(ThreatData{u, area});
            continue;
        }

        const double area_loss = area-count2050;
        RestorationPlan<MutableLandscape>::Option option = plan.addOption(area_loss);
        plan.addNode(option, u, area_loss);
    }

    io::CSVReader<3> links("data/quebec_leam_v3/raw/aretes_leam_v3.csv");
    links.read_header(io::ignore_extra_column, "from", "to", "Dist");
    int from, to;
    double Dist;
    while(links.read_row(from, to, Dist)) {
        Graph_t::Node u = node_correspondance[from-1];
        Graph_t::Node v = node_correspondance[to-1];
        if(u == lemon::INVALID || v == lemon::INVALID) continue;
        double probability = p(Dist);
        if(probability < thresold) continue;
        landscape.addArc(u, v, probability);
        landscape.addArc(v, u, probability);
    }

    for(ThreatData data : threaten_list) {
        Graph_t::Node v1 = data.node;
        RestorationPlan<MutableLandscape>::Option option = plan.addOption(data.area);
        Graph_t::Node v2 = landscape.addNode(0, landscape.getCoords(v1) + Point(0.001, 0.001));
        
        for(Graph_t::OutArcIt a(graph, v1), next_a = a; a != lemon::INVALID; a = next_a) {
            ++next_a;
            landscape.changeSource(a, v2);
        }
        Graph_t::Arc v1v2 = landscape.addArc(v1, v2, 0.5);
        plan.addArc(option, v1v2, 1);
        plan.addNode(option, v2, data.area);
    }
    
    return instance;
}


Instance make_instance_biorevaix_level_1(const double restoration_coef=2, const Point center=Point(897286.5,6272835.5), const double radius=200) {
    Instance instance;
    MutableLandscape & landscape = instance.landscape;
    const Graph_t & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape>& plan = instance.plan;

    auto prob = [] (const double cost) {
        return cost == 1 ? 1
            : cost == 10 ? 0.98
            : cost == 150 ? 0.8
            : cost == 300 ? 0.6
            : cost == 800 ? 0.4
            : 0;
    };

    std::array<Graph_t::Node, 688402> nodes;
    nodes.fill(lemon::INVALID);
    Graph_t::NodeMap<double> node_prob(graph, 0.0);
    Graph_t::NodeMap<RestorationPlan<MutableLandscape>::Option> troncon_option(graph);
    std::array<RestorationPlan<MutableLandscape>::Option, 5195> id_tronc_option;
    id_tronc_option.fill(-1);

    const double radius_squared = radius*radius;
    io::CSVReader<5> patches("../landscape_opt_datas/BiorevAix/raw/vertexN1_v2.csv");
    patches.read_header(io::ignore_extra_column, "N1_id", "X", "Y", "cost", "id_tronc2");
    int N_id, id_tronc;
    double X, Y, cost;
    while(patches.read_row(N_id, X, Y, cost, id_tronc)) {
        Point p = Point(X,Y)-center;
        if(p.normSquare() > radius_squared) continue;
        if(cost == 1000) continue;
        Graph_t::Node u = landscape.addNode((cost==1 ? 1 : 0), p);
        nodes[N_id] = u;
        node_prob[u] = prob(cost);
        troncon_option[u] = -1;
        if(id_tronc == 0) continue;
        RestorationPlan<MutableLandscape>::Option & option = id_tronc_option[id_tronc];
        if(option == -1) option = plan.addOption(0);
        troncon_option[u] = option;
        plan.setCost(option, plan.getCost(option) + 1);
    }

    io::CSVReader<2> links("../landscape_opt_datas/BiorevAix/raw/AL_N1.csv");
    links.read_header(io::ignore_extra_column, "from", "to");
    int from, to;
    while(links.read_row(from, to)) {
        Graph_t::Node u = nodes[from];
        Graph_t::Node v = nodes[to];
        if(u == lemon::INVALID || v == lemon::INVALID) continue;
        double probability = std::sqrt(node_prob[u]*node_prob[v]);
        probability = std::max(std::min(probability, 1.0), 0.0);

        if(probability == 0) continue;

        Graph_t::Arc uv = landscape.addArc(u, v, probability);
        Graph_t::Arc vu = landscape.addArc(v, u, probability);

        RestorationPlan<MutableLandscape>::Option option_u = troncon_option[u];
        RestorationPlan<MutableLandscape>::Option option_v = troncon_option[v];
        if(option_u < 0 && option_v < 0) continue;
        assert(option_u < 0 || option_v < 0 || option_u == option_v);
        RestorationPlan<MutableLandscape>::Option option = std::max(option_u, option_v);

        double restored_prob = std::pow(node_prob[u], 1/(2 * (option_u >= 0 ? restoration_coef : 1)))
            * std::pow(node_prob[v], 1/(2 * (option_v >= 0 ? restoration_coef : 1)));

        if(restored_prob <= probability) continue;

        plan.addArc(option, uv, restored_prob);
        plan.addArc(option, vu, restored_prob);
    }
    return instance;
}

Instance make_instance_biorevaix_level_1_area_2(const double restoration_coef=2) {
    Instance instance;
    MutableLandscape & landscape = instance.landscape;
    const Graph_t & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape>& plan = instance.plan;

    auto prob = [] (const double cost) {
        return cost == 1 ? 1
            : cost == 10 ? 0.98
            : cost == 150 ? 0.8
            : cost == 300 ? 0.6
            : cost == 800 ? 0.4
            : 0;
    };

    std::array<Graph_t::Node, 688402> nodes;
    nodes.fill(lemon::INVALID);
    Graph_t::NodeMap<double> node_prob(graph, 0.0);
    Graph_t::NodeMap<RestorationPlan<MutableLandscape>::Option> troncon_option(graph);
    std::array<RestorationPlan<MutableLandscape>::Option, 5195> id_tronc_option;
    id_tronc_option.fill(-1);

    io::CSVReader<6> patches("../landscape_opt_datas/BiorevAix/raw/vertexN1_v2.csv");
    patches.read_header(io::ignore_extra_column, "N1_id", "X", "Y", "area2", "cost", "id_tronc2");
    int N_id, area2, id_tronc;
    double X, Y, cost;
    while(patches.read_row(N_id, X, Y, area2, cost, id_tronc)) {
        if(!area2) continue;
        if(cost == 1000) continue;
        Graph_t::Node u = landscape.addNode((cost==1 ? 1 : 0), Point(X,Y));
        nodes[N_id] = u;
        node_prob[u] = prob(cost);
        troncon_option[u] = -1;
        if(id_tronc == 0) continue;
        RestorationPlan<MutableLandscape>::Option & option = id_tronc_option[id_tronc];
        if(option == -1) option = plan.addOption(0);
        troncon_option[u] = option;
        plan.setCost(option, plan.getCost(option) + 1);
    }

    io::CSVReader<2> links("../landscape_opt_datas/BiorevAix/raw/AL_N1.csv");
    links.read_header(io::ignore_extra_column, "from", "to");
    int from, to;
    while(links.read_row(from, to)) {
        Graph_t::Node u = nodes[from];
        Graph_t::Node v = nodes[to];
        if(u == lemon::INVALID || v == lemon::INVALID) continue;
        double probability = std::sqrt(node_prob[u]*node_prob[v]);
        probability = std::max(std::min(probability, 1.0), 0.0);

        if(probability == 0) continue;

        Graph_t::Arc uv = landscape.addArc(u, v, probability);
        Graph_t::Arc vu = landscape.addArc(v, u, probability);

        RestorationPlan<MutableLandscape>::Option option_u = troncon_option[u];
        RestorationPlan<MutableLandscape>::Option option_v = troncon_option[v];
        if(option_u < 0 && option_v < 0) continue;
        assert(option_u < 0 || option_v < 0 || option_u == option_v);
        RestorationPlan<MutableLandscape>::Option option = std::max(option_u, option_v);

        double restored_prob = std::pow(node_prob[u], 1/(2 * (option_u >= 0 ? restoration_coef : 1)))
            * std::pow(node_prob[v], 1/(2 * (option_v >= 0 ? restoration_coef : 1)));

        if(restored_prob <= probability) continue;

        plan.addArc(option, uv, restored_prob);
        plan.addArc(option, vu, restored_prob);
    }
    return instance;
}



Instance make_instance_biorevaix_level_2_v7(const double restoration_coef=2) {
    Instance instance;
    MutableLandscape & landscape = instance.landscape;
    const Graph_t & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape>& plan = instance.plan;

    auto prob = [] (const double cost) {
        return cost == 1 ? 1
            : cost == 10 ? 0.98
            : cost == 150 ? 0.8
            : cost == 300 ? 0.6
            : cost == 800 ? 0.4
            : 0;
    };

    std::array<Graph_t::Node, 688402> nodes;
    nodes.fill(lemon::INVALID);
    Graph_t::NodeMap<double> node_prob(graph, 0.0);
    Graph_t::NodeMap<RestorationPlan<MutableLandscape>::Option> troncon_option(graph);
    std::array<RestorationPlan<MutableLandscape>::Option, 5195> id_tronc_option;
    id_tronc_option.fill(-1);

    io::CSVReader<5> patches("../landscape_opt_datas/BiorevAix/vertexN2_v7.txt");
    patches.read_header(io::ignore_extra_column, "N2_id", "X", "Y", "area2", "cost_mode");
    int N_id, area2;
    double X, Y, cost;
    while(patches.read_row(N_id, X, Y, area2, cost)) {
        // if(!area2) continue;
        // if(cost == 1000) continue;
        Graph_t::Node u = landscape.addNode((cost==1 ? 1 : 0), Point(X,Y));
        nodes[N_id] = u;
        node_prob[u] = prob(cost);
    }

    io::CSVReader<2> links("../landscape_opt_datas/BiorevAix/AL_N2.txt");
    links.read_header(io::ignore_extra_column, "from", "to");
    int from, to;
    while(links.read_row(from, to)) {
        Graph_t::Node u = nodes[from];
        Graph_t::Node v = nodes[to];
        if(u == lemon::INVALID || v == lemon::INVALID) continue;
        double probability = std::sqrt(node_prob[u]*node_prob[v]);
        probability = std::max(std::min(probability, 1.0), 0.0);

        // if(probability == 0) continue;

        Graph_t::Arc uv = landscape.addArc(u, v, probability);
        Graph_t::Arc vu = landscape.addArc(v, u, probability);
    }
    return instance;
}



Instance make_instance_biorevaix_level_2(const double restoration_coef=2) {
    Instance instance;
    MutableLandscape & landscape = instance.landscape;
    const Graph_t & graph = landscape.getNetwork();
    RestorationPlan<MutableLandscape>& plan = instance.plan;

    auto prob = [] (const double cost) {
        return cost == 1 ? 0.999
            : cost == 10 ? 0.98
            : cost == 150 ? 0.8
            : cost == 300 ? 0.6
            : cost == 800 ? 0.4
            : 0;
    };

    std::array<Graph_t::Node, 98344> nodes;
    nodes.fill(lemon::INVALID);
    std::array<int, 98344> node_tronc;
    Graph_t::NodeMap<double> node_prob(graph, 0.0);
    Graph_t::NodeMap<RestorationPlan<MutableLandscape>::Option> troncon_option(graph);
    std::array<RestorationPlan<MutableLandscape>::Option, 5195> id_tronc_option;
    id_tronc_option.fill(-1);


    int N2_id, id_tronc;
    double X, Y, cost;

    io::CSVReader<2> N1_patches("../landscape_opt_datas/BiorevAix/raw/vertexN1_v2.csv");
    N1_patches.read_header(io::ignore_extra_column, "N2_id", "id_tronc2");
    while(N1_patches.read_row(N2_id, id_tronc)) {
        node_tronc[N2_id] = std::max(node_tronc[N2_id], id_tronc);
    }

    io::CSVReader<4> patches("../landscape_opt_datas/BiorevAix/raw/cropped_vertexN2.csv");
    patches.read_header(io::ignore_extra_column, "N2_id", "X", "Y", "cost_mean");
    while(patches.read_row(N2_id, X, Y, cost)) {
        if(cost == 1000) continue;
        id_tronc = node_tronc[N2_id];
        Graph_t::Node u = landscape.addNode((cost==1 ? 1 : 0), Point(X,Y));
        nodes[N2_id] = u;
        node_prob[u] = prob(cost);
        troncon_option[u] = -1;
        if(id_tronc == 0) continue;
        RestorationPlan<MutableLandscape>::Option & option = id_tronc_option[id_tronc];
        if(option == -1) option = plan.addOption(0);
        troncon_option[u] = option;
        plan.setCost(option, plan.getCost(option) + 1);
    }

    io::CSVReader<2> links("../landscape_opt_datas/BiorevAix/raw/cropped_AL_N2.csv");
    links.read_header(io::ignore_extra_column, "from", "to");
    int from, to;
    while(links.read_row(from, to)) {
        Graph_t::Node u = nodes[from];
        Graph_t::Node v = nodes[to];
        if(u == lemon::INVALID || v == lemon::INVALID) continue;
        double probability = std::sqrt(node_prob[u]*node_prob[v]);
        probability = std::max(std::min(probability, 1.0), 0.0);

        if(probability == 0) continue;

        Graph_t::Arc uv = landscape.addArc(u, v, probability);
        Graph_t::Arc vu = landscape.addArc(v, u, probability);

        RestorationPlan<MutableLandscape>::Option option_u = troncon_option[u];
        RestorationPlan<MutableLandscape>::Option option_v = troncon_option[v];
        if(option_u < 0 && option_v < 0) continue;
        assert(option_u < 0 || option_v < 0 || option_u == option_v);
        RestorationPlan<MutableLandscape>::Option option = std::max(option_u, option_v);

        double restored_prob = std::pow(node_prob[u], 1/(2 * (option_u >= 0 ? restoration_coef : 1)))
            * std::pow(node_prob[v], 1/(2 * (option_v >= 0 ? restoration_coef : 1)));

        plan.addArc(option, uv, restored_prob);
        plan.addArc(option, vu, restored_prob);
    }
    return instance;
}


#endif // INSTANCES_HELPER_HPP