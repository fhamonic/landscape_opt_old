#include "helper.hpp"

void Helper::printSolution(const Landscape & landscape, const RestorationPlan<Landscape>& plan, std::string name, concepts::Solver & solver, double B, const Solution & solution) {
    const Graph_t & graph = landscape.getNetwork();
        
        auto radius = [&] (double area) { return std::sqrt(area / (2*M_PI)); };
        Graph_t::NodeMap<int> node_idsMap(graph);
        Graph_t::NodeMap<lemon::Color> node_colorsMap(graph, lemon::WHITE);
        Graph_t::NodeMap<double> node_sizesMap(graph, 0);
        for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
            node_idsMap[v] = graph.id(v);
            node_sizesMap[v] = radius(landscape.getQuality(v));
        }

        double node_scale = 0.8;
        double text_scale = 0.75;
        double arrow_scale = (1-node_scale)*2/3;
        
        const auto & [r_max, min_dist] = findNodeScale(landscape);
        const double a_max = r_max*2*radius(minNonZeroQuality(landscape));

        Graph_t::ArcMap<lemon::Color> arcs_colorsMap(graph, lemon::BLACK);
        Graph_t::ArcMap<double> arc_widths(graph, 0.001);


        std::string str_B = std::to_string(B);
        str_B.erase ( str_B.find_last_not_of('0') + 1, std::string::npos );
        
        std::string out_eps = name + "_B=" + str_B + "_" + solver.toString();
        std::string title = std::to_string(solution.getComputeTimeMs()) + " ms, ECA = " +
                std::to_string(ECA::get().eval_solution(landscape, plan, solution)) + " cost = " + std::to_string(solution.getCost());

        std::cout << title << std::endl;

        for(Graph_t::NodeIt u(graph); u != lemon::INVALID; ++u) {
            double coef = 0;
            for(const auto & e : plan[u])
                coef = std::max(coef, solution[e.option]);
            node_colorsMap[u] = lemon::Color(1-coef, coef, 0.0);
        }
        for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
            double coef = 0;
            for(const auto & e : plan[a])
                coef = std::max(coef, solution[e.option]);
            arcs_colorsMap[a] = lemon::Color(1-coef, coef, 0.0);
        }

        return lemon::graphToEps(graph, "output/" + name)
                .title(name)
                .coords(landscape.getCoordsMap())
                .autoNodeScale(false)
                .absoluteNodeSizes(true)
                .nodeSizes(node_sizesMap)
                .nodeScale(node_scale * r_max)
                .autoArcWidthScale(false)
                .absoluteArcWidths(true)
                .arcWidths(arc_widths)
                .arcWidthScale(1)
                .drawArrows(true)
                .arrowLength(arrow_scale * min_dist)
                .arrowWidth(arrow_scale * min_dist)
                .nodeTexts(node_idsMap)
                .nodeTextSize(text_scale * a_max)
                .nodeColors(node_colorsMap)
                .arcColors(arcs_colorsMap)
                .enableParallel(false)
                .parArcDist(2 * arrow_scale * min_dist)
                .border(20)
                .run();
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
    return (p == p) && (p >= 0) && (p <= 1);
}

void Helper::assert_well_formed(const Landscape & landscape, const RestorationPlan<Landscape>& plan) {
    const Graph_t & graph = landscape.getNetwork();

    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        assert(landscape.getQuality(v)==landscape.getQuality(v) && landscape.getQuality(v) >= 0); }
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
        assert(is_probability(landscape.getProbability(a))); }

    for(Graph_t::NodeIt v(graph); v != lemon::INVALID; ++v) {
        for(auto const & e : plan[v]) {
            assert(plan.contains(e.id));
            assert(e.quality_gain > 0.0);
        }
    }
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
        for(auto const & e : plan[a]) {
            assert(plan.contains(e.id));
            assert(is_probability(e.restored_probability));
            assert(e.restored_probability > landscape.getProbability(a));
        }
    }
}