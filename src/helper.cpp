#include "helper.hpp"

void Helper::printSolution(const Landscape & landscape, std::string name, concepts::Solver & solver, double B, Solution * solution) {
    const Graph_t & graph = landscape.getNetwork();
    
    
    //*
    auto radius = [&] (Graph_t::Node u) { return std::sqrt(landscape.getQuality(u) / (2*M_PI)); };
    Graph_t::NodeMap<int> node_idsMap(graph);
    Graph_t::NodeMap<lemon::Color> node_colorsMap(graph, lemon::WHITE);
    Graph_t::NodeMap<double> node_sizesMap(graph);
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        node_idsMap[v] = graph.id(v);
        node_sizesMap[v] = radius(v);
    }
    Graph_t::ArcMap<lemon::Color> arcs_colorsMap(graph, lemon::BLACK);
    Graph_t::ArcMap<double> arc_widths(graph, 5);

    for(auto option_pair : solution->getOptionCoefs()) {
        const double coef = option_pair.second;
        for(Graph_t::Node u : option_pair.first->nodes()) {
            node_colorsMap[u] = lemon::Color(1-coef, coef, 0.0);
        }
        for(Graph_t::Arc a : option_pair.first->arcs()) {
            arcs_colorsMap[a] = lemon::Color(1-coef, coef, 0.0);
        }
    }
    double node_scale = 0.9;
    /*/
    Graph_t::NodeMap<int> node_idsMap(graph);
    Graph_t::NodeMap<lemon::Color> node_colorsMap(graph, lemon::WHITE);
    Graph_t::NodeMap<double> node_sizesMap(graph, 1);
    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v){
        node_idsMap[v] = graph.id(v);
        node_sizesMap[v] = std::log(landscape.getQuality(v)+2);
    }

    Graph_t::ArcMap<lemon::Color> arcs_colorsMap(graph, lemon::WHITE);
    Graph_t::ArcMap<double> arc_widths(graph, 0);

    for(auto option_pair : solution->getOptionCoefs()) {
        const double coef = option_pair.second;
        for(Graph_t::Arc a : option_pair.first->arcs()) {
            node_colorsMap[graph.source(a)] = lemon::Color(1-coef, coef, 0.0);
            break;
        }
    }
    double node_scale = 3;
    //*/
    
    //TODO ECA::eval_solution(landscape, alpha, solution)
    const double base_eca = ECA::get().eval(landscape);
    const double restored_eca = ECA::get().eval_solution(landscape, *solution);
    
    const double cost = solution->getCost();
    const double time_ms = solution->getComputeTimeMs();

    std::cout << "base ECA : " << base_eca << std::endl 
            << "restored ECA : " << restored_eca << std::endl 
            << "ECA gain : " << restored_eca - base_eca << std::endl 
            << "cost : " << cost << std::endl 
            << "obj : " << solution->obj << std::endl 
            << "time_ms : " << time_ms << std::endl;

    std::string str_B = std::to_string(B);
    str_B.erase ( str_B.find_last_not_of('0') + 1, std::string::npos );
    
    std::string out_eps = name + "_B=" + str_B + "_" + solver.toString();
    std::string title = (solution == NULL ? "Fail" : std::to_string(time_ms)) + " ms, ECA = " + std::to_string(restored_eca) + " cost = " + std::to_string(cost);

    const double r_max = findNodeScale(landscape);
    const double a_max = 2*r_max*M_PI;

    return lemon::graphToEps(graph, "output/" + name)
            .title(title)
            .coords(landscape.getCoordsMap())
            .autoNodeScale(false)
            .absoluteNodeSizes(true)
            .nodeSizes(node_sizesMap)
            //.nodeScale(0.9 * r_max)
            .nodeScale(node_scale * r_max)
            .autoArcWidthScale(false)
            .absoluteArcWidths(true)
            .arcWidths(arc_widths)
            .drawArrows(true)
            .arrowLength(50)
            .arrowWidth(r_max / 6)
            .nodeTexts(node_idsMap)
            .nodeTextSize(a_max / 4)
            .nodeColors(node_colorsMap)
            .arcColors(arcs_colorsMap)
            .enableParallel(true)
            .parArcDist(r_max / 4)
            .border(20)
            .run();
}

void Helper::copyPlan(RestorationPlan & contracted_plan, const RestorationPlan & plan, const Graph_t::NodeMap<Graph_t::Node> & nodesRef, const Graph_t::ArcMap<Graph_t::Arc> & arcsRef) {
    for(RestorationPlan::Option * option : plan.options()) {
        RestorationPlan::Option * copied_option = contracted_plan.addOption();
        copied_option->setId(option->getId());
        copied_option->setCost(option->getCost());
        for(Graph_t::Node u : option->nodes())
            copied_option->addPatch(nodesRef[u], option->getQualityGain(u));
        for(Graph_t::Arc a : option->arcs())
            copied_option->addLink(arcsRef[a], option->getRestoredProbability(a));
    }  
}


// need to include the binary search tree for y-h , y+h search
std::pair<Graph_t::Node, Graph_t::Node> Helper::neerestNodes(const Landscape & landscape) {
    const Graph_t & graph = landscape.getNetwork();
    const Landscape::CoordsMap & coordsMap = landscape.getCoordsMap();

    auto dist = [&coordsMap](Graph_t::Node a, Graph_t::Node b){
        const double dx = coordsMap[b].x - coordsMap[a].x;
        const double dy = coordsMap[b].y - coordsMap[a].y;
        return dx*dx + dy*dy;
    };

    std::vector<Graph_t::Node> nodes;
    for (Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u)
        nodes.push_back(u);

    std::sort(nodes.begin(), nodes.end(), [&coordsMap](Graph_t::Node a, Graph_t::Node b) {
        return coordsMap[a].x < coordsMap[b].x;
    });

    std::list<Graph_t::Node> near_line;
    near_line.push_back(nodes[0]);
    near_line.push_back(nodes[1]);
    double min_distance = dist(nodes[0], nodes[1]);
    std::pair<Graph_t::Node, Graph_t::Node> neerestPair(nodes[0], nodes[1]);

    const int n = nodes.size();
    for(int i=2; i<n; i++) {
        const Graph_t::Node p = nodes[i];
        const double current_x = coordsMap[p].x;
        while(coordsMap[near_line.front()].x < current_x - min_distance)
            near_line.pop_front();
        for(Graph_t::Node u : near_line) {
            const double d_up = dist(u, p);
            if(min_distance < d_up)
                continue;
            neerestPair = std::pair<Graph_t::Node, Graph_t::Node>(u,p);
        }
        near_line.push_back(p);
    }
    return neerestPair;
}


bool is_probability(const double p) {
    return (p >= 0) && (p <= 1);
}

void Helper::assert_well_formed(const Landscape & landscape, const RestorationPlan & plan) {
    const Graph_t & graph = landscape.getNetwork();

    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v)
        assert(landscape.getQuality(v) >= 0);
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a)
        assert(is_probability(landscape.getProbability(a)));

    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        for(RestorationPlan::Option * option : plan.getOptions(v)) {
            const std::vector<RestorationPlan::Option*> & opt_list = plan.options();
            assert(std::find(opt_list.begin(),opt_list.end(),option) != opt_list.end());
        }
    }
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
        for(RestorationPlan::Option * option : plan.getOptions(a)) {
            const std::vector<RestorationPlan::Option*> & opt_list = plan.options();
            assert(std::find(opt_list.begin(),opt_list.end(),option) != opt_list.end());
        }
    }

    //std::cout << plan.getNbOptions() << std::endl;

    for(RestorationPlan::Option * option : plan.options()) {
        for(Graph_t::Node u : option->nodes()) {
            assert(graph.valid(u));
            assert(option->contains(u));
            assert(option->getQualityGain(u) > 0);
            assert(option->id(u) >= 0 && option->id(u) < option->getNbNodes());
            assert(plan.contains(u));
            const std::list<RestorationPlan::Option*> & opt_list = plan.getOptions(u);
            assert(std::find(opt_list.begin(),opt_list.end(),option) != opt_list.end());
        }
        for(Graph_t::Arc a : option->arcs()) {
            assert(graph.valid(a));
            assert(option->contains(a));
            //std::cout << graph.id(graph.source(a)) << " " << option->getRestoredProbability(a) << std::endl;
            assert(is_probability(option->getRestoredProbability(a)));
            assert(option->getRestoredProbability(a) > landscape.getProbability(a));
            assert(option->id(a) >= 0 && option->id(a) < option->getNbArcs());
            assert(plan.contains(a));
            const std::list<RestorationPlan::Option*> & opt_list = plan.getOptions(a);
            assert(std::find(opt_list.begin(),opt_list.end(),option) != opt_list.end());
        }
    }
}