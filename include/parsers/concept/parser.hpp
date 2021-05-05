#ifndef PARSER_HPP
#define PARSER_HPP

#include<iostream>
#include <filesystem>

namespace concepts {
    template <typename T>
    class Parser {
        protected:
            Parser() {};
        public:
            ~Parser() {};
            virtual T parse(std::filesystem::path file_path)=0;
    };
}

#endif //PARSER_HPP