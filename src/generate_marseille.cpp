#include <iostream>
#include <filesystem>
#include <fstream>

#include "indices/eca.hpp"
#include "random_chooser.hpp"

#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

int main (int argc, const char *argv[]) {
    if(argc < 3) {
        std::cerr << "input requiered : <landscape_file> <nb_friches>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];
    const int wanted_nb_friches = std::atoi(argv[2]);

    Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);
    const Graph_t & graph = landscape->getNetwork();

    RandomChooser<Graph_t::Node> friches_chooser;

    int cpt = 0;
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        if(landscape->getQuality(v) == 1) {
            friches_chooser.add(v, 1.0);
            cpt++;
        }
    }
    assert(cpt > wanted_nb_friches);
    for(int i=0; i < (cpt - wanted_nb_friches); i++)
        landscape->removeNode(friches_chooser.pick());

    StdLandscapeParser::get().write(*landscape, "output", "marseille_analysis", true);

    return EXIT_SUCCESS;
}

/*
int main (int argc, const char *argv[]) {
    if(argc < 6) {
        std::cerr << "input requiered : <landscape_file> <thresold> <length_gain> <number_friches> <restore_friches>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];
    const int thresold = std::atoi(argv[2]);
    const int length_gain = std::atoi(argv[3]);
    const int nb_friches = std::atoi(argv[4]);
    const int restore_friches = std::atoi(argv[5]);

    Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);
    const Graph_t & graph = landscape->getNetwork();

    std::vector<Graph_t::Arc> arcs_to_delete;
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a)
        if(landscape->getDifficulty(a) > thresold)
            arcs_to_delete.push_back(a);
    for(Graph_t::Arc a : arcs_to_delete)
        landscape->removeLink(a);


    RandomChooser<Graph_t::Node> friches_chooser;

    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        if(landscape->getQuality(v) <= 0.1) {
            friches_chooser.add(v, 1.0);
            landscape->setQuality(v, 0.0);
        }
    }

    std::vector<Graph_t::Node> friches;
    for(int i=0; i<nb_friches; i++)
        friches.push_back(friches_chooser.pick());
    while(friches_chooser.canPick())
        landscape->removePatch(friches_chooser.pick());


    StdLandscapeParser::get().write(*landscape, "output", "marseille_" + std::to_string(thresold), true);

    RestorationPlan true_plan(*landscape);

    for(Graph_t::Node v : friches) {
        RestorationPlan::Option * option = true_plan.addOption();
        option->setCost(1);

        for(Graph_t::InArcIt a(graph,v); a != lemon::INVALID; ++a) {
            double trimed_gain = std::min((double)length_gain, landscape->getDifficulty(a) - 0.000001);
            option->addLink(a, trimed_gain);
        }
        for(Graph_t::OutArcIt a(graph,v); a != lemon::INVALID; ++a) {
            double trimed_gain = std::min((double)length_gain, landscape->getDifficulty(a) - 0.000001);
            option->addLink(a, trimed_gain);
        }

        if(restore_friches > 0)
            option->addPatch(v, 0.1);
    }
    
    StdRestorationPlanParser parser(*landscape);
    parser.write(true_plan, "output", "marseille_" + std::to_string(thresold) + "_" + std::to_string(length_gain), true);

    return EXIT_SUCCESS;
}*/

/*
echo "set terminal pdf;
set output 'output/bench.pdf';
set style line 1 linecolor rgb '#ff0033' linetype 1 linewidth 1;
set style line 2 linecolor rgb '#0080b3' linetype 1 linewidth 1;
set style line 3 linecolor rgb '#00ff7f' linetype 1 linewidth 1;
set style line 4 linecolor rgb '#ff6699' linetype 1 linewidth 1; set style line 5 linecolor rgb '#ffff00' linetype 1 linewidth 1; set style line 6 linecolor rgb '#800080' linetype 1 linewidth 1; set key right bottom
; plot 'output/test_100.csv' using 1:3 ti '100' with lines linestyle 1, 'output/test_200.csv' using 1:3 ti '200' with lines linestyle 2, 'output/test_500.csv' using 1:3 ti '500' with lines linestyle 3, 'output/test_1000.csv' using 1:3 ti '1000' with lines linestyle 4;" | gnuplot
*/

/*

echo "set terminal pdf;
set output 'output/bench.pdf';
set style line 1 linecolor rgb '#ff0033' linetype 1 linewidth 1;
set style line 2 linecolor rgb '#0080b3' linetype 1 linewidth 1;
set style line 3 linecolor rgb '#00ff7f' linetype 1 linewidth 1;
set style line 4 linecolor rgb '#ff6699' linetype 1 linewidth 1;
set key right bottom;
plot 'output/test.csv' using 1:2 ti 'variables arc lengths -20%' with lines linestyle 1, 'output/test.csv' using 1:3 ti 'contraintes arc lengths -20%' with lines linestyle 2, 'output/test_500.csv' using 1:2 ti 'variables arc lengths -50%' with lines linestyle 3, 'output/test_500.csv' using 1:3 ti 'contraintes arc lengths -50%' with lines linestyle 4" | gnuplot
*/
