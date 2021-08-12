#ifndef STD_LANDSCAPE_PARSER_HPP
#define STD_LANDSCAPE_PARSER_HPP

#include <fstream>
#include <iostream>

#include "landscape/mutable_landscape.hpp"
#include "parsers/concept/parser.hpp"

#include "fast-cpp-csv-parser/csv.h"

class StdMutableLandscapeParser : public concepts::Parser<MutableLandscape> {
private:
    static StdMutableLandscapeParser singleton;
    StdMutableLandscapeParser();

public:
    static StdMutableLandscapeParser & get() noexcept { return singleton; }
    ~StdMutableLandscapeParser();

    MutableLandscape parse(const std::filesystem::path file_name);
    void write(const MutableLandscape & landscape,
               const std::filesystem::path output, const std::string name,
               bool use_range_ids = true);
};

#endif  // STD_LANDSCAPE_PARSER_HPP