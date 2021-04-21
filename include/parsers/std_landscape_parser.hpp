#ifndef STD_LANDSCAPE_PARSER_HPP
#define STD_LANDSCAPE_PARSER_HPP

#include <iostream>
#include <fstream>

#include "parsers/concept/parser.hpp"
#include "landscape/landscape.hpp"

#include "fast-cpp-csv-parser/csv.h"

class StdLandscapeParser : public concepts::Parser<Landscape> {
    private:
        static StdLandscapeParser singleton;
        StdLandscapeParser();
    public:
        static StdLandscapeParser & get() noexcept {
            return singleton;
        }
        ~StdLandscapeParser();
   
        Landscape * parse(const std::filesystem::path file_name);
        void write(const Landscape & landscape, const std::filesystem::path output, const std::string name, bool use_range_ids=true);
};

#endif //STD_LANDSCAPE_PARSER_HPP