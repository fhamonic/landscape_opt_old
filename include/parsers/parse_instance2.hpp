#ifndef INSTANCE2_PARSER_HPP
#define INSTANCE2_PARSER_HPP

#include <filesystem>
#include <fstream>
#include <iostream>

#include "solvers/concept/instance2.hpp"

#include <fast-cpp-csv-parser/csv.h>
#include <nlohmann/json.hpp>

Instance2 parse_instance2(std::filesystem::path instance_path) {
    Instance2 instance;

    std::ifstream instance_stream(instance_path);
    nlohmann::json instance_json;
    instance_stream >> instance_json;

    if(!instance_json.contains("options"))
        throw std::invalid_argument("No 'options' property in " +
                                    instance_path.string());

    auto options_json = instance_json["options"];
    if(!instance_json.contains("options"))
        throw std::invalid_argument("No 'file' property in 'options' of " +
                                    instance_path.string());
    std::filesystem::path options_csv_path = options_json["file"];
    if(options_csv_path.is_relative())
        options_csv_path = (instance_path.parent_path() / options_csv_path);
    std::string options_id_column = "id";
    std::string options_cost_column = "cost";
    if(options_json.contains("columns")) {
        auto options_columns_json = options_json["columns"];
        if(options_columns_json.contains("id"))
            options_id_column = options_columns_json["id"];
        if(options_columns_json.contains("cost"))
            options_cost_column = options_columns_json["cost"];
    }

    io::CSVReader<2> options_csv(options_csv_path);
    options_csv.read_header(io::ignore_extra_column, options_id_column,
                            options_cost_column);

    std::string option_id;
    double option_cost;
    while(options_csv.read_row(option_id, option_cost)) {
        if(instance.containsOption(option_id))
            throw std::invalid_argument("Option identifier '" + option_id +
                                        "' appears multiple times in ");
        instance.addOption(option_id, option_cost);
    }

    return instance;
}

#endif  // INSTANCE2_PARSER_HPP