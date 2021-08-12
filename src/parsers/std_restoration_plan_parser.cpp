#include "parsers/std_restoration_plan_parser.hpp"

StdRestorationPlanParser::StdRestorationPlanParser(const MutableLandscape & l)
    : landscape(l) {}
StdRestorationPlanParser::~StdRestorationPlanParser() {}

RestorationPlan<MutableLandscape> StdRestorationPlanParser::parse(
    std::filesystem::path file_path) {
    RestorationPlan<MutableLandscape> plan(landscape);
    const MutableLandscape::Graph & graph = landscape.getNetwork();

    if(!std::filesystem::exists(file_path)) {
        std::cerr << file_path << ":"
                  << " File does not exists" << std::endl;
        assert(false);
    }

    std::ifstream file(file_path);

    int cpt = 0;

    auto assert_node = [&file_path, &cpt, &graph](int id,
                                                  MutableLandscape::Node u) {
        if(graph.valid(u)) return true;
        std::cerr << file_path << ":" << cpt << " Invalid node " << id
                  << std::endl;
        return false;
    };
    auto assert_arc = [&file_path, &cpt, &graph](int id_source, int id_target,
                                                 MutableLandscape::Arc a) {
        if(graph.valid(a)) return true;
        std::cerr << file_path << ":" << cpt << " Invalid arc (" << id_source
                  << "," << id_target << ") " << std::endl;
        return false;
    };
    auto unexpected_eof = [&file_path, &cpt, &file]() {
        if(!file.eof()) return false;
        std::cerr << file_path << ":" << cpt << " Unexpected EOF" << std::endl;
        return true;
    };

    double cost;
    while(file >> cost) {
        int nb_elems;

        file >> nb_elems;
        if(unexpected_eof()) {
            assert(false);
        }

        RestorationPlan<MutableLandscape>::Option option = plan.addOption(cost);

        for(int i = 0; i < nb_elems; i++) {
            char type;

            if(unexpected_eof()) {
                assert(false);
            }
            file >> type;
            if(unexpected_eof()) {
                assert(false);
            }

            if(type == 'n') {
                int id;
                double quality;
                file >> id;
                if(unexpected_eof()) {
                    assert(false);
                }
                file >> quality;
                MutableLandscape::Node u = graph.nodeFromId(id);
                if(!assert_node(id, u)) {
                    assert(false);
                }
                plan.addNode(option, u, quality);
                continue;
            }
            if(type == 'a') {
                int id_source, id_target;
                double length;
                file >> id_source;
                if(unexpected_eof()) {
                    assert(false);
                }
                file >> id_target;
                if(unexpected_eof()) {
                    assert(false);
                }
                file >> length;
                MutableLandscape::Node source = graph.nodeFromId(id_source);
                if(!assert_node(id_source, source)) {
                    assert(false);
                }
                MutableLandscape::Node target = graph.nodeFromId(id_target);
                if(!assert_node(id_target, target)) {
                    assert(false);
                }
                MutableLandscape::Arc a = lemon::findArc(graph, source, target);
                if(!assert_arc(id_source, id_target, a)) {
                    assert(false);
                }
                plan.addArc(option, a, length);
                continue;
            }
        }
        cpt++;
    }

    return plan;
}

bool StdRestorationPlanParser::write(
    const RestorationPlan<MutableLandscape> & plan,
    const std::filesystem::path output, const std::string name,
    bool use_range_ids) {
    const MutableLandscape & landscape = plan.getLandscape();
    const MutableLandscape::Graph & graph = landscape.getNetwork();

    std::ofstream problem_file(output / (name + ".problem"));

    auto nodes_rangeIdMap = lemon::rangeIdMap<MutableLandscape::Node>(graph);

    auto id = [&graph, &nodes_rangeIdMap,
               use_range_ids](MutableLandscape::Node u) {
        if(use_range_ids) return nodes_rangeIdMap[u];
        return graph.id(u);
    };

    problem_file << std::setprecision(16);

    const auto nodeOptions = plan.computeNodeOptionsMap();
    const auto arcOptions = plan.computeArcOptionsMap();

    for(RestorationPlan<MutableLandscape>::Option i = 0;
        i < plan.getNbOptions(); ++i) {
        problem_file << plan.getCost(i) << " "
                     << (nodeOptions[i].size() + arcOptions[i].size())
                     << std::endl;

        for(auto const & [v, quality_gain] : nodeOptions[i])
            problem_file << "\tn " << id(v) << " " << quality_gain << std::endl;
        for(auto const & [a, restored_probability] : arcOptions[i]) {
            MutableLandscape::Node source = graph.source(a);
            MutableLandscape::Node target = graph.target(a);
            problem_file << "\ta " << id(source) << " " << id(target) << " "
                         << restored_probability << std::endl;
        }
    }

    return true;
}