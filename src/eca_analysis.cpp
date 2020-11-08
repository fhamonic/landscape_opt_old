#include <iostream>
#include <filesystem>
#include <fstream>

#include "lemon/dijkstra.h"

#include "indices/eca.hpp"

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"


RestorationPlan<Landscape>* getPlan(const Landscape & landscape, double length_gain, double quality_gain) {
    const Graph_t & graph = landscape.getNetwork();
    RestorationPlan<Landscape>* plan = new RestorationPlan(landscape);
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        if(landscape.getQuality(v) > 0.1)
            continue;
        RestorationPlan<Landscape>::Option* option = plan->addOption();
        option->setCost(1);
        if(length_gain > 0)
            for(Graph_t::OutArcIt a(graph,v); a != lemon::INVALID; ++a) {
                double trimed_gain = std::min((double)length_gain, landscape.getDifficulty(a) - 0.00001);
                option->addLink(a, trimed_gain);
            }
        if(quality_gain > 0)
            option->addPatch(v, quality_gain);
    }
    return plan;
}

void seuiller(Landscape & landscape, const double thresold) {
    const Graph_t & graph = landscape.getNetwork();

    std::vector<Graph_t::ArcIt> arcs_to_delete;
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a)
        if(landscape.getDifficulty(a) >= thresold)
            arcs_to_delete.push_back(a);
    for(Graph_t::ArcIt a : arcs_to_delete)
        landscape.removeLink(a);
}

int main (int argc, const char *argv[]) {
    if(argc < 2) {
        std::cerr << "input requiered : <landscape_file>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];

    Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);

    const Graph_t & graph = landscape->getNetwork();

    Graph_t::NodeMap<bool> massifs(graph, false);
    Graph_t::NodeMap<bool> parcs(graph, false);
    Graph_t::NodeMap<bool> friches(graph, false);
    Graph_t::NodeMap<bool> massifs_or_parcs(graph, false);

    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        const int quality = landscape->getQuality(v);
        switch(quality) {
            case 3:
                massifs[v] = true;
                landscape->setQuality(v, 10);
                break;
            case 25:
                parcs[v] = true;
                landscape->setQuality(v, 5);
                break;
            case 21:
                parcs[v] = true;
                landscape->setQuality(v, 1);
                break;
            case 1:
                friches[v] = true;
                landscape->setQuality(v, 0);
                break;
            default:
                assert(false);
        }
        massifs_or_parcs[v] = parcs[v] || massifs[v];
    }



    std::ofstream data_log("output/data.log");
    data_log << std::fixed << std::setprecision(3);

    ECA & eca = ECA::get();

    data_log << "thresold " <<
            "length_gain " <<
            "area_gain " <<
            "alpha " <<
            "total_eca " <<
            "massifs_eca " <<
            "parcs_eca " <<
            "massifs_parcs_eca" << std::endl;

    std::vector<double> thresold_values{3000}; 
    std::vector<double> alpha_values{500, 1000, 2000, 4000}; 
    std::vector<double> length_gain_values{30, 100, 200, 300}; 
    std::vector<double> area_gain_values{0, 0.5}; 

    for(int thresold : thresold_values) {
        seuiller(*landscape, thresold);
        RestorationPlan<Landscape>* plan = getPlan(*landscape, 0.0, 0.5);

        for(int length_gain : length_gain_values) {
            for(int area_gain : area_gain_values) {

                RestorationPlan<Landscape>* plan = getPlan(*landscape, length_gain, area_gain);

                DecoredLandscape decored_landscape(*landscape);
                for(RestorationPlan<Landscape>::Option* option : plan->options())
                    decored_landscape.apply(option);

                delete plan;

                for(int alpha : alpha_values) {
                    const double total_eca = pow(eca.eval(*landscape, alpha), 2);
                    const double massifs_eca = pow(eca.eval(*landscape, alpha, massifs), 2);
                    const double parcs_eca = pow(eca.eval(*landscape, alpha, parcs), 2);
                    const double friches_eca = pow(eca.eval(*landscape, alpha, friches), 2);

                    const double parcs_massifs_eca = pow(eca.eval(*landscape, alpha, massifs_or_parcs), 2) - massifs_eca - parcs_eca;

                    data_log << thresold << " " 
                                    << length_gain << " " 
                                    << area_gain << " " 
                                    << alpha << " "
                                    << total_eca << " "
                                    << parcs_eca << " "
                                    << massifs_eca << " "
                                    << parcs_eca << " "
                                    << friches_eca << " "
                                    << parcs_massifs_eca << std::endl;
                }
            }
        }
    }

    delete landscape;

    return EXIT_SUCCESS;
}
