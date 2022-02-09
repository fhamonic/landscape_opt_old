#ifndef INSTANCE2_PARSER_HPP
#define INSTANCE2_PARSER_HPP

#include <filesystem>
#include <fstream>
#include <iostream>

#include "solvers/concept/instance2.hpp"

#include <fast-cpp-csv-parser/csv.h>
#include <nlohmann/json.hpp>

template <typename T>
void assert_json_property(T json_object, std::string property,
                          std::string parent = "") {
    if(!instance_json.contains("options"))
        throw std::invalid_argument(
            "No 'file' property" +
            (parent.empty() ? "" : " in '" + parent + "'"));
}

std::string json_type_str(nlhomann::detail::value_t type) {
    switch(type) {
        case nlhomann::detail::value_t::null:
            return "null";
        case nlhomann::detail::value_t::object:
            return "object";
        case nlhomann::detail::value_t::array:
            return "array";
        case nlhomann::detail::value_t::string:
            return "string";
        case nlhomann::detail::value_t::boolean:
            return "boolean";
        case nlhomann::detail::value_t::number_integer:
            return "number_integer";
        case nlhomann::detail::value_t::number_unsigned:
            return "number_unsigned";
        case nlhomann::detail::value_t::number_float:
            return "number_float";
        case nlhomann::detail::value_t::binary:
            return "binary";
    }
}

template <typename T>
void assert_json_properties(
    T json_object,
    std::initializer_list<std::string, nlhomann::detail::value_t> properties) {
    for(const auto & [property, type] : properties) {
        if(!instance_json.contains("options"))
            throw std::invalid_argument("No 'file' property");
        if(!instance_json.type() != type)
            throw std::invalid_argument(
                "Property" + property + " expected to of type '" +
                json_type_str(type) + "' but is '" +
                json_type_str(instance_json.type()) + "'");
    }
}

Instance2 parse_instance2(std::filesystem::path instance_path) {
    Instance2 instance;

    std::ifstream instance_stream(instance_path);
    nlohmann::json instance_json;
    instance_stream >> instance_json;

    instance_json.type()

        assert_json_property(instance_json, "options");
    auto options_json = instance_json["options"];
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

    if(!instance_json.contains("cases"))
        throw std::invalid_argument("No 'file' property in 'cases' of " +
                                    instance_path.string());

    return instance;
}

#endif  // INSTANCE2_PARSER_HPP