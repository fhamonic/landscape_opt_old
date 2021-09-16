#include <algorithm>
#include <execution>
#include <iostream>
#include <vector>

#include <atomic>
#include <thread>
#include "tbb/global_control.h"

int main(int argc, char ** argv) {
    if(argc < 2) {
        std::cout << "Usage: " << argv[0] << " <N>" << std::endl;
        return EXIT_FAILURE;
    }
    const std::size_t n = std::atoi(argv[1]);
    std::vector<int> v;
    v.reserve(n);
    for(int i =0; i< n; ++i) {
        v.push_back(i);
    }

    std::atomic<int> sum = 0;
    std::for_each(std::execution::par_unseq, v.begin(), v.end(),
                  [&](const auto i) { sum += i; });

    std::cout << sum << std::endl;

    return EXIT_SUCCESS;
}