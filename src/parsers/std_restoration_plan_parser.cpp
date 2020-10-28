#include "parsers/std_restoration_plan_parser.hpp"

StdRestorationPlanParser::StdRestorationPlanParser(const Landscape & l) : landscape(l) {}
StdRestorationPlanParser::~StdRestorationPlanParser() {}
   
RestorationPlan * StdRestorationPlanParser::parse(std::filesystem::path file_path) {
    if(!std::filesystem::exists(file_path)) {
        std::cerr << file_path << ":" << " File does not exists" << std::endl;
        return nullptr;
    }

    RestorationPlan * plan = new RestorationPlan(landscape);
    const Graph_t & graph = landscape.getNetwork();

    std::ifstream file(file_path);

    int cpt = 0;

    auto assert_node = [&file_path,&cpt,&graph] (int id, Graph_t::Node u) {
        if(graph.valid(u))
            return true;
        std::cerr << file_path << ":" << cpt << " Invalid node " << id << std::endl;
        return false;
    };
    auto assert_arc = [&file_path,&cpt,&graph] (int id_source, int id_target, Graph_t::Arc a) {
        if(graph.valid(a))
            return true;
        std::cerr << file_path << ":" << cpt << " Invalid arc (" << id_source << "," << id_target << ") " << std::endl;
        return false;
    };
    auto unexpected_eof = [&file_path,&cpt,&file] () {
        if(!file.eof())
            return false;
        std::cerr << file_path << ":" << cpt << " Unexpected EOF" << std::endl;
        return true;
    };

    double cost;
    while(file >> cost) {
        int nb_elems;

        file >> nb_elems; if(unexpected_eof()) { delete plan; return nullptr; }

        RestorationPlan::Option option = plan->addOption(cost);

        for(int i=0; i<nb_elems; i++) {
            char type;

            if(unexpected_eof()) { delete plan; return nullptr; }
            file >> type;
            if(unexpected_eof()) { delete plan; return nullptr; }

            if(type == 'n') {
                int id;
                double quality;
                file >> id; if(unexpected_eof()) { delete plan; return nullptr; }
                file >> quality;
                Graph_t::Node u = graph.nodeFromId(id);
                if(!assert_node(id, u)) { delete plan; return nullptr; }
                plan->addNode(option, u, quality);
                continue;
            }
            if(type == 'a') {
                int id_source, id_target;
                double length;
                file >> id_source; if(unexpected_eof()) { delete plan; return nullptr; }
                file >> id_target; if(unexpected_eof()) { delete plan; return nullptr; }
                file >> length;
                Graph_t::Node source = graph.nodeFromId(id_source); if(!assert_node(id_source, source)) { delete plan; return nullptr; }
                Graph_t::Node target = graph.nodeFromId(id_target); if(!assert_node(id_target, target)) { delete plan; return nullptr; }
                Graph_t::Arc a = lemon::findArc(graph, source, target); if(!assert_arc(id_source, id_target, a)) { delete plan; return nullptr; }
                plan->addArc(option, a, length);
                continue;
            }
        }
        cpt++;
    }

    return plan;
}

bool StdRestorationPlanParser::write(const RestorationPlan & plan, const std::filesystem::path output, const std::string name, bool use_range_ids) {
    const Landscape & landscape = plan.getLandscape();
    const Graph_t & graph = landscape.getNetwork();

    std::ofstream problem_file(output / (name + ".problem"));

    auto nodes_rangeIdMap = lemon::rangeIdMap<Graph_t::Node>(graph);

    auto id = [&graph,&nodes_rangeIdMap,use_range_ids](Graph_t::Node u){
        if(use_range_ids)
            return nodes_rangeIdMap[u];
        return graph.id(u);
    };

    problem_file << std::setprecision(16);

    for(RestorationPlan::Option i=0; i<plan.getNbOptions(); ++i) {
        problem_file << plan.getCost(i) << " " << plan.getNbElements(i) << std::endl;

        for(auto const& [v, quality_gain] : plan.getNodes(i)) {
            problem_file << "\tn " << id(v) << " " << quality_gain << std::endl;
        }
        for(auto const& [a, restored_probability] : plan.getArcs(i)) {
            Graph_t::Node source = graph.source(a);
            Graph_t::Node target = graph.target(a);
            problem_file << "\ta " << id(source) << " " << id(target) << " " << restored_probability << std::endl;
        }
    }

    return true;
}