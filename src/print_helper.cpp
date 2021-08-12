#include "print_helper.hpp"

void Helper::printSolution(const MutableLandscape & landscape, const RestorationPlan<MutableLandscape>& plan, std::string name, concepts::Solver & solver, double B, const Solution & solution) {
    const MutableLandscape::Graph & graph = landscape.getNetwork();
        
    auto radius = [&] (double area) { return std::sqrt(area / (2*M_PI)); };
    MutableLandscape::Graph::NodeMap<int> node_idsMap(graph);
    MutableLandscape::Graph::NodeMap<lemon::Color> node_colorsMap(graph, lemon::WHITE);
    MutableLandscape::Graph::NodeMap<double> node_sizesMap(graph, 0);
    for(MutableLandscape::NodeIt v(graph); v != lemon::INVALID; ++v) {
        node_idsMap[v] = graph.id(v);
        node_sizesMap[v] = radius(landscape.getQuality(v));
    }

    double node_scale = 0.8;
    double text_scale = 0.75;
    double arrow_scale = (1-node_scale)*2/3;
    
    const auto & [r_max, min_dist] = findNodeScale(landscape);
    const double a_max = r_max*2*radius(minNonZeroQuality(landscape));

    MutableLandscape::Graph::ArcMap<lemon::Color> arcs_colorsMap(graph, lemon::BLACK);
    MutableLandscape::Graph::ArcMap<double> arc_widths(graph, 0.001);


    std::string str_B = std::to_string(B);
    str_B.erase ( str_B.find_last_not_of('0') + 1, std::string::npos );
    
    std::string out_eps = name + "_B=" + str_B + "_" + solver.toString();
    std::string title = std::to_string(solution.getComputeTimeMs()) + " ms, ECA = " +
            std::to_string(ECA().eval(Helper::decore_landscape(landscape, plan, solution))) + " cost = " + std::to_string(solution.getCost());

    std::cout << title << std::endl;

    for(MutableLandscape::NodeIt u(graph); u != lemon::INVALID; ++u) {
        double coef = 0;
        for(const auto & e : plan[u])
            coef = std::max(coef, solution[e.option]);
        node_colorsMap[u] = lemon::Color(1-coef, coef, 0.0);
    }
    for(MutableLandscape::ArcIt a(graph); a != lemon::INVALID; ++a) {
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
std::pair<MutableLandscape::Node, MutableLandscape::Node> Helper::neerestNodes(const MutableLandscape & landscape) {
    const MutableLandscape::Graph & graph = landscape.getNetwork();
    const MutableLandscape::CoordsMap & coordsMap = landscape.getCoordsMap();

    auto dist = [&coordsMap](MutableLandscape::Node a, MutableLandscape::Node b){
        const double dx = coordsMap[b].x - coordsMap[a].x;
        const double dy = coordsMap[b].y - coordsMap[a].y;
        return dx*dx + dy*dy;
    };

    std::vector<MutableLandscape::Node> nodes;
    for (MutableLandscape::NodeIt u(graph); u != lemon::INVALID; ++u)
        nodes.push_back(u);

    std::sort(nodes.begin(), nodes.end(), [&coordsMap](MutableLandscape::Node a, MutableLandscape::Node b) {
        return coordsMap[a].x < coordsMap[b].x;
    });

    std::list<MutableLandscape::Node> near_line;
    near_line.push_back(nodes[0]);
    near_line.push_back(nodes[1]);
    double min_distance = dist(nodes[0], nodes[1]);
    std::pair<MutableLandscape::Node, MutableLandscape::Node> neerestPair(nodes[0], nodes[1]);

    const int n = nodes.size();
    for(int i=2; i<n; i++) {
        const MutableLandscape::Node p = nodes[i];
        const double current_x = coordsMap[p].x;
        while(coordsMap[near_line.front()].x < current_x - min_distance)
            near_line.pop_front();
        for(MutableLandscape::Node u : near_line) {
            const double d_up = dist(u, p);
            if(min_distance < d_up)
                continue;
            neerestPair = std::pair<MutableLandscape::Node, MutableLandscape::Node>(u,p);
        }
        near_line.push_back(p);
    }
    return neerestPair;
}