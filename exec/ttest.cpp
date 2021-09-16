#include <filesystem>
#include <fstream>
#include <iostream>

#include <thread>
#include "tbb/global_control.h"

int main() {
    std::cout << "max allowed parallelism : "
              << std::thread::hardware_concurrency() << std::endl;
    return EXIT_SUCCESS;
}