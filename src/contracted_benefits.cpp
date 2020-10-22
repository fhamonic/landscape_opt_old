#include <iostream>
#include <filesystem>
#include <fstream>

#include "lemon/dijkstra.h"
#include "lemon/connectivity.h"

#include "lemon/graph_to_eps.h"


#include "parsers/std_restoration_plan_parser.hpp"
#include "parsers/std_landscape_parser.hpp"

#include "utils/random_chooser.hpp"

#include "helper.hpp"

#include "precomputation/my_contraction_algorithm.hpp"

Landscape * make_landscape(const Landscape & landscape, Graph_t::Node center, int nb_nodes) {
    const Graph_t & graph = landscape.getNetwork();

    Graph_t::NodeMap<Graph_t::Node> nodes(graph, lemon::INVALID);

    Landscape * new_landscape = new Landscape();
    const Graph_t & new_graph = new_landscape->getNetwork();
        
    const Graph_t::ArcMap<double> & difficultyMap = landscape.getDifficultyMap();
    lemon::Dijkstra<Graph_t, Graph_t::ArcMap<double>> dijkstra(graph, difficultyMap);
    dijkstra.init();
    dijkstra.addSource(center);

    int cpt = 0;
    while (!dijkstra.emptyQueue()) {
        Graph_t::Node x = dijkstra.processNextNode();
        if(cpt >= nb_nodes)
            continue;
        nodes[x] = new_landscape->addPatch(landscape.getQuality(x), landscape.getCoords(x));
        cpt++;
    }
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
        Graph_t::Node u = nodes[graph.source(a)];
        Graph_t::Node v = nodes[graph.target(a)];
        if(! (new_graph.valid(u) && new_graph.valid(v)))
            continue;
        new_landscape->addLink(u, v, landscape.getDifficulty(a));
    }

    return new_landscape;
}

int countVariables(const RestorationPlan & plan) {
    int sum = 0;
    for(RestorationPlan::Option * option : plan.options()) {
        sum += option->getNbNodes();
        sum += option->getNbArcs();
    }
    return sum;
}
int countConstraints(const RestorationPlan & plan) {
    int sum = 0;
    for(RestorationPlan::Option * option : plan.options()) {
        sum += option->getNbNodes();
        sum += option->getNbArcs();
    }
    return sum;
}

int main (int argc, const char *argv[]) {
    if(argc < 2) {
        std::cerr << "input requiered : <landscape_file>" << std::endl;
        return EXIT_FAILURE;
    }
    std::filesystem::path landscape_path = argv[1];

    const double alpha = 6000;
    
    Landscape * landscape = StdLandscapeParser::get().parse(landscape_path);
    const Graph_t & graph = landscape->getNetwork();

    // StdLandscapeParser::get().write(*landscape, "output", "out_landscape");

    MyContractionAlgorithm alg2;

    RandomChooser<Graph_t::Node> node_chooser;
    for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u)
        node_chooser.add(u, 1);

    const int nb_tests = 10;
    const int n_to_test[] = {500};
    
    std::vector<double> percent_vars;
    std::vector<double> percent_constraints;

    for(int i=0; i<nb_tests; i++) {
        Graph_t::Node center = node_chooser.pick();

        int cpt = 0;

        std::cout << "step " << i << "/" << nb_tests << std::endl;

        for(int n : n_to_test) {
            Landscape * sub_landscape = make_landscape(*landscape, center, n);
            const Graph_t & sub_graph = sub_landscape->getNetwork();

            const int m = lemon::countArcs(sub_graph);

            RandomChooser<Graph_t::Arc> arc_chooser;
            for(Graph_t::ArcIt a(sub_graph); a != lemon::INVALID; ++a)
                arc_chooser.add(a, 1);

            for(int percent_arcs=0; percent_arcs<=100; percent_arcs+=5) {
                RestorationPlan plan(*sub_landscape);
                const int nb_restored_arcs = percent_arcs * m / 100;
                arc_chooser.reset();
                for(int id_option=0; id_option<nb_restored_arcs; id_option++) {
                    Graph_t::Arc a = arc_chooser.pick();
                    RestorationPlan::Option * option = plan.addOption();
                    option->setId(id_option);
                    option->setCost(1);
                    option->addLink(a, sub_landscape->getDifficulty(a) / 2);
                }

                std::chrono::time_point<std::chrono::high_resolution_clock> last_time, current_time;
                last_time = std::chrono::high_resolution_clock::now();

                Graph_t::NodeMap<ContractionResult> * results = alg2.precompute(*sub_landscape, plan, alpha);
                
                current_time = std::chrono::high_resolution_clock::now();
                int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time-last_time).count();

                int var_count = n * (m + countVariables(plan)) + n + plan.getNbOptions();
                int constraint_count = n * (n + countConstraints(plan)) + 1;

                // COUNT CONTRACTED VARS AND CONSTRAINTS
                int var_count_contracted = 0;
                int constraint_count_contracted = 0;
                for(Graph_t::NodeIt u(sub_graph); u != lemon::INVALID; ++u) {
                    ContractionResult & cr = (*results)[u];
                    int n_contracted = lemon::countNodes(cr.landscape->getNetwork());
                    int m_contracted = lemon::countArcs(cr.landscape->getNetwork());

                    var_count_contracted += m_contracted + countVariables(*cr.plan);
                    constraint_count_contracted += n_contracted + countConstraints(*cr.plan);
                }
                var_count_contracted += n + plan.getNbOptions();
                constraint_count_contracted += 1;

                for(Graph_t::NodeIt u(sub_graph); u != lemon::INVALID; ++u) {
                    delete (*results)[u].landscape;
                    delete (*results)[u].plan;
                }
                delete results;

                // std::cout << n << " " 
                //         << m << " "
                //         << percent_arcs << " "
                //         << var_count << " "
                //         << constraint_count << " "
                //         << var_count_contracted << " "
                //         << constraint_count_contracted << " "
                //         << time_ms << std::endl;

                const double percent_vars_contracted = (double)var_count_contracted / var_count * 100;
                const double percent_constraints_contracted = (double)constraint_count_contracted / constraint_count * 100;

                if(i == 0) percent_vars.push_back(percent_vars_contracted);
                else percent_vars[cpt] += percent_vars_contracted;   
                
                if(i == 0) percent_constraints.push_back(percent_constraints_contracted);
                else percent_constraints[cpt] += percent_constraints_contracted;               
                

                // std::cout << n << "_" << m << " "
                //         << percent_arcs << " "
                //         << percent_vars_contracted << " "
                //         << percent_constraints_contracted << std::endl;

                cpt++;
            }

            delete sub_landscape;
        }
    }
    
    int cpt = 0;
    for(int n : n_to_test) {
        for(int percent_arcs=0; percent_arcs<=100; percent_arcs+=5) {
            std::cout << n << " "
                        << percent_arcs << " "
                        << percent_vars[cpt] / nb_tests << " "
                        << percent_constraints[cpt] / nb_tests << std::endl;
            cpt++;
        }
    }

    delete landscape;

    return EXIT_SUCCESS;
}

/*
echo "set terminal pdf;
set output 'percent_variables.pdf';
set style line 1 linecolor rgb '#ff0033' linetype 1 linewidth 1;
set style line 2 linecolor rgb '#0080b3' linetype 1 linewidth 1;
set style line 3 linecolor rgb '#00ff7f' linetype 1 linewidth 1;
set style line 4 linecolor rgb '#ff6699' linetype 1 linewidth 1; 
set key right bottom;
set title 'percentage of threatened arcs vs percentage of variables after contraction'
set ylabel 'percentage of variables' textcolor black;
set xlabel 'percentage of threatened arcs' textcolor black;
plot 'size_100.csv' using 1:2 ti '100 patchs graphs' with lines linestyle 1, 'size_200.csv' using 1:2 ti '200 patchs graphs' with lines linestyle 2, 'size_500.csv' using 1:2 ti '500 patchs graphs' with lines linestyle 3, 'size_1000.csv' using 1:2 ti '1000 patchs graphs' with lines linestyle 4;" | gnuplot
*/
/*
echo "set terminal pdf;
set output 'percent_constraints.pdf';
set style line 1 linecolor rgb '#ff0033' linetype 1 linewidth 1;
set style line 2 linecolor rgb '#0080b3' linetype 1 linewidth 1;
set style line 3 linecolor rgb '#00ff7f' linetype 1 linewidth 1;
set style line 4 linecolor rgb '#ff6699' linetype 1 linewidth 1; 
set key right bottom;
set title 'percentage of threatened arcs vs percentage of constraints after contraction'
set ylabel 'percentage of constraints' textcolor black;
set xlabel 'percentage of threatened arcs' textcolor black;
plot 'size_100.csv' using 1:3 ti '100 patchs graphs' with lines linestyle 1, 'size_200.csv' using 1:3 ti '200 patchs graphs' with lines linestyle 2, 'size_500.csv' using 1:3 ti '500 patchs graphs' with lines linestyle 3, 'size_1000.csv' using 1:3 ti '1000 patchs graphs' with lines linestyle 4;" | gnuplot
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
