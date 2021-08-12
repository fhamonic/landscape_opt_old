#ifndef PARSER_HPP
#define PARSER_HPP

#include <filesystem>
#include <iostream>

namespace concepts {
template <typename T>
class Parser {
protected:
    Parser(){};

public:
    ~Parser(){};
    virtual T parse(std::filesystem::path file_path) = 0;
};
}  // namespace concepts

#endif  // PARSER_HPP