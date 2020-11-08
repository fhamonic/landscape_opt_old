#ifndef STD_RESTORATION_PLAN_PARSER_HPP
#define STD_RESTORATION_PLAN_PARSER_HPP

#include <iostream>
#include <fstream>

#include "parsers/concept/parser.hpp"
#include "solvers/concept/restoration_plan.hpp"

#include"landscape/landscape.hpp"

class StdRestorationPlanParser : public concepts::Parser<RestorationPlan<Landscape>> {
    private:
        const Landscape & landscape;
    public:
        StdRestorationPlanParser(const Landscape & l);
        ~StdRestorationPlanParser();
   
        RestorationPlan<Landscape>* parse(std::filesystem::path file_path);
        bool write(const RestorationPlan<Landscape>& plan, const std::filesystem::path output, const std::string name, bool use_range_ids=true);
};

#endif //STD_RESTORATION_PLAN_PARSER_HPP