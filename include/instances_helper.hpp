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
#include <math.h>
#include <random>

#include "landscape/landscape.hpp"
#include "solvers/concept/restoration_plan.hpp"

#include "csv.hpp"
#include "utils/random_chooser.hpp"


class Instance {
    public:
        Landscape landscape;
        const Graph_t & graph;
        RestorationPlan<Landscape> plan;

        Instance() : graph(landscape.getNetwork()), plan(landscape) {}
};


void addCostNoise(Instance & instance, double deviation_ratio=0.2, int seed=456) {
    std::default_random_engine generator(seed);
    std::normal_distribution<double> distribution(1.0, deviation_ratio);

    auto noise = [&generator, &distribution] (double value) {
        return std::max(std::numeric_limits<double>::epsilon(), value * distribution(generator));
    };

    for(RestorationPlan<Landscape>::Option i=0; i<instance.plan.getNbOptions(); ++i)
        instance.plan.setCost(i, noise(instance.plan.getCost(i)));
}


Instance * make_instance_marseillec(double pow, double thresold, double median, int nb_friches=100) {
    Instance * instance = new Instance;
    
    Landscape & landscape = instance->landscape;
    const Graph_t & graph = instance->graph;
    RestorationPlan<Landscape> & plan = instance->plan;

    auto d = [&landscape] (Graph_t::Node u, Graph_t::Node v) { return std::sqrt((landscape.getCoords(u) - landscape.getCoords(v)).normSquare()); };
    auto p = [median, pow] (const double d) { return std::exp(std::pow(d,pow)/std::pow(median, pow)*std::log(0.5)); };

    typedef struct { Point p; double area; double price; Graph_t::Node node; } FricheData;
    RandomChooser<FricheData> friches_chooser(9876);
    std::vector<FricheData> friches_list;

    io::CSVReader<5> patches("data/Marseille/vertices_marseillec.csv");
    patches.read_header(io::ignore_extra_column,"category","x","y","area2","price_rel");
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
        RestorationPlan<Landscape>::Option option = plan.addOption(data.price);
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


Instance * make_instance_quebec(double pow, double thresold, double median,
        Point orig=Point(240548, 4986893), Point dim=Point(32360, 20000)) {
    Instance * instance = new Instance;
    
    Landscape & landscape = instance->landscape;
    const Graph_t & graph = instance->graph;
    RestorationPlan<Landscape>& plan = instance->plan;

    auto p = [median, pow] (const double d) { return std::exp(std::pow(d,pow)/std::pow(median, pow)*std::log(0.5)); };
    
    std::vector<Graph_t::Node> node_correspondance;
    typedef struct { Graph_t::Node node; double area; } ThreatData;
    std::vector<ThreatData> threaten_list;

    io::CSVReader<4> patches("data/quebec_leam_v3/raw/sommets_leam_v3.csv");
    patches.read_header(io::ignore_extra_column, "area","xcoord","ycoord","count2050");
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
        RestorationPlan<Landscape>::Option option = plan.addOption(area_loss);
        plan.addNode(option, u, area_loss);
    }

    io::CSVReader<3> links("data/quebec_leam_v3/raw/aretes_leam_v3.csv");
    links.read_header(io::ignore_extra_column, "from","to","Dist");
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
        RestorationPlan<Landscape>::Option option = plan.addOption(data.area);
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

#endif // INSTANCES_HELPER_HPP