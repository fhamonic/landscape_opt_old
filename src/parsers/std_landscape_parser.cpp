#include "parsers/std_landscape_parser.hpp"

StdLandscapeParser StdLandscapeParser::singleton;
   
StdLandscapeParser::StdLandscapeParser() {}
StdLandscapeParser::~StdLandscapeParser() {}
   
Landscape * StdLandscapeParser::parse(const std::filesystem::path file_name) {
    io::CSVReader<2> landscape_files(file_name);
    landscape_files.read_header(io::ignore_extra_column, "patches_file", "links_file");
    
    std::string patches_file, links_file;
    if(! landscape_files.read_row(patches_file, links_file)){
        std::cerr << "StdLandscapeParser : failed reading " << file_name << std::endl;
        return NULL;
    }

    io::CSVReader<4> patches(patches_file);
    patches.read_header(io::ignore_extra_column, "id", "weight", "x", "y");
    
    io::CSVReader<3> links(links_file);
    links.read_header(io::ignore_extra_column, "source_id", "target_id", "probability");

    Landscape * landscape = new Landscape();
    const Graph_t & g = landscape->getNetwork();

    int patch_id;
    double patch_weight, patch_x, patch_y;
    while(patches.read_row(patch_id, patch_weight, patch_x, patch_y)) {
        const Graph_t::Node & new_node = landscape->addNode(patch_weight, Point(patch_x, patch_y));
        if(g.id(new_node) != patch_id) {
            std::cerr << "StdLandscapeParser : Warning in file " << patches_file << " line " << patches.get_file_line() << " : expexted id " << g.id(new_node) << " but was " << patch_id << "." << std::endl;
            delete landscape;
            return NULL;
        }
    }

    int link_source_id, link_target_id;
    double link_probability;
    while(links.read_row(link_source_id, link_target_id, link_probability)) {
        if(link_source_id < 0 || link_source_id > patch_id) {
            std::cerr << "StdLandscapeParser : Warning in file " << links_file << " line " << links.get_file_line() << " : invalid patch id : " << link_source_id << "." << std::endl;
            delete landscape;
            return NULL;
        }
        if(link_target_id < 0 || link_target_id > patch_id) {
            std::cerr << "StdLandscapeParser : Warning in file " << links_file << " line " << links.get_file_line() << " : invalid patch id : " << link_target_id << "." << std::endl;
            delete landscape;
            return NULL;
        }

        const Graph_t::Node & u = g.nodeFromId(link_source_id);
        const Graph_t::Node & v = g.nodeFromId(link_target_id);
        landscape->addArc(u, v, link_probability);
    }

    return landscape;
}

void StdLandscapeParser::write(const Landscape & landscape, const std::filesystem::path output, const std::string name, bool use_range_ids) {
    const Graph_t & graph = landscape.getNetwork();

    std::ofstream index_file(output / (name + ".index"));
    std::ofstream patchs_file(output / (name + ".patchs"));
    std::ofstream links_file(output / (name + ".links"));

    index_file << "patches_file,links_file" << std::endl << (output / (name + ".patchs")).generic_string() << "," << (output / (name + ".links")).generic_string();

    const int n = lemon::countNodes(graph);
    assert(use_range_ids || graph.maxNodeId() == n-1);

    auto nodes_rangeIdMap = lemon::rangeIdMap<Graph_t::Node>(graph);
    auto reverse_nodes_rangeIdMap = nodes_rangeIdMap.inverse();

    auto nodeFromId = [&graph,&reverse_nodes_rangeIdMap,use_range_ids] (int node_id) {
        if(use_range_ids)
            return reverse_nodes_rangeIdMap[node_id];
        return graph.nodeFromId(node_id);
    };
    auto id = [&graph,&nodes_rangeIdMap,use_range_ids](Graph_t::Node u){
        if(use_range_ids)
            return nodes_rangeIdMap[u];
        return graph.id(u);
    };

    patchs_file << "id,weight,x,y" << std::setprecision(16) << std::endl;
    for(int i=0; i<n; i++) {
        Graph_t::Node u = nodeFromId(i);
        patchs_file << i << "," << landscape.getQuality(u) << "," << landscape.getCoords(u).x << "," << landscape.getCoords(u).y << std::endl;
    }    
    
    links_file << "source_id,target_id,length" << std::setprecision(16) << std::endl;
    for(Graph_t::ArcIt a(graph); a != lemon::INVALID; ++a) {
        Graph_t::Node u = graph.source(a);
        Graph_t::Node v = graph.target(a);
        links_file << id(u) << "," << id(v) << "," << landscape.getProbability(a) << std::endl;
    }
}