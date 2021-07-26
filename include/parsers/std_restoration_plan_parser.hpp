#ifndef STD_RESTORATION_PLAN_PARSER_HPP
#define STD_RESTORATION_PLAN_PARSER_HPP

#include <iostream>
#include <fstream>

#include "parsers/concept/parser.hpp"
#include "solvers/concept/restoration_plan.hpp"

#include"landscape/mutable_landscape.hpp"

class StdRestorationPlanParser : public concepts::Parser<RestorationPlan<MutableLandscape>> {
    private:
        const MutableLandscape & landscape;
    public:
        StdRestorationPlanParser(const MutableLandscape & l);
        ~StdRestorationPlanParser();
   
        RestorationPlan<MutableLandscape> parse(std::filesystem::path file_path);
        bool write(const RestorationPlan<MutableLandscape> & plan, const std::filesystem::path output, const std::string name, bool use_range_ids=true);
};

#endif //STD_RESTORATION_PLAN_PARSER_HPP