#include <iostream>
#include <filesystem>

// #include "csv_ctre.hpp"
#include "ctre.hpp"

static constexpr auto pattern = ctll::fixed_string{ "h.*" };
constexpr auto match(std::string_view sv) noexcept {
    return ctre::match<pattern>(sv);
}

int main() {
    
    std::cout << match("hello world") << std::endl;
    return EXIT_SUCCESS;
}