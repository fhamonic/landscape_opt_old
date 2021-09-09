cmake_minimum_required(VERSION 3.12)

project(
    Landscape_Opt
    VERSION 1.0
    DESCRIPTION
        "Library for handling ecological landscape datas and optimize their connectivity according to the PC indicator .. and more incomming"
    LANGUAGES CXX)

# set(CMAKE_VERBOSE_MAKEFILE ON)

# ################### Options ####################
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(WARNINGS "" OFF)
option(HARDCORE_WARNINGS "" OFF)
option(COMPILE_FOR_NATIVE "" OFF)
option(COMPILE_WITH_LTO "" OFF)
option(WITH_GUROBI "" OFF)

set(WARNINGS_LIST "-Wall;-Wextra;")
set(HARDCORE_WARNINGS_LIST
    "-Wall;-Wextra;-Wshadow;-Wpedantic;-Wnon-virtual-dtor;-Wold-style-cast;-Wcast-align;-Wunused;-Woverloaded-virtual;-Wpedantic;-Wconversion;-Wsign-conversion;-Wmisleading-indentation;-Wduplicated-cond;-Wduplicated-branches;-Wlogical-op;-Wnull-dereference;-Wuseless-cast;-Wdouble-promotion;-Wformat=2;-Wlifetime"
)

# ################### Modules ####################
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(CMakeBuildTypeUtils)

# ################### Packages ####################
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(LEMON REQUIRED)
find_package(COIN REQUIRED)
if(${WITH_GUROBI} STREQUAL "ON")
    find_package(GUROBI REQUIRED)
endif()

# #################### Sources ######################
file(GLOB SRC "src/*.cpp" "src/*/*.cpp" "src/*/*/*.cpp" "src/*/*/*/*.cpp")

# ################### LIB target ####################
add_library(landscape_opt ${SRC})

target_include_directories(landscape_opt PUBLIC include)
target_include_directories(landscape_opt PUBLIC thirdparty)

target_link_libraries(landscape_opt CONAN_PKG::fast-cpp-csv-parser)
target_link_libraries(landscape_opt CONAN_PKG::tbb)
target_link_libraries(landscape_opt CONAN_PKG::boost)
target_link_libraries(landscape_opt CONAN_PKG::fmt)
target_link_libraries(landscape_opt CONAN_PKG::eigen)
target_link_libraries(landscape_opt CONAN_PKG::range-v3)

target_include_directories(landscape_opt PUBLIC ${LEMON_INCLUDE_DIRS})
target_link_libraries(landscape_opt ${LEMON_LIBRARIES})
target_link_libraries(landscape_opt ${COIN_LIBRARIES})

if(${WITH_GUROBI} STREQUAL "ON")
    target_include_directories(landscape_opt PUBLIC ${GUROBI_INCLUDE_DIR})
    target_link_libraries(landscape_opt ${GUROBI_LIBRARIES})
endif()

target_compile_options_if_exists(landscape_opt PRIVATE ${WARNINGS}
                                 ${WARNINGS_LIST})
target_compile_options_if_exists(landscape_opt PRIVATE ${HARDCORE_WARNINGS}
                                 ${HARDCORE_WARNINGS_LIST})
target_compile_options_if_exists(landscape_opt PRIVATE ${COMPILE_FOR_NATIVE}
                                 -march=native)
target_compile_options_if_exists(landscape_opt PRIVATE ${COMPILE_WITH_LTO}
                                 -flto)

# ################## EXEC target ####################
add_executable(solve exec/solve.cpp)
target_include_directories(solve PUBLIC include)
target_include_directories(solve PUBLIC thirdparty)
target_link_libraries(solve PUBLIC landscape_opt)

add_executable(quebec_analysis exec/quebec_analysis.cpp)
target_include_directories(quebec_analysis PUBLIC include)
target_include_directories(quebec_analysis PUBLIC thirdparty)
target_link_libraries(quebec_analysis PUBLIC landscape_opt)

add_executable(marseille_analysis exec/marseille_analysis.cpp)
target_include_directories(marseille_analysis PUBLIC include)
target_include_directories(marseille_analysis PUBLIC thirdparty)
target_link_libraries(marseille_analysis PUBLIC landscape_opt)

add_executable(eval_eca exec/eval_eca.cpp)
target_include_directories(eval_eca PUBLIC include)
target_include_directories(eval_eca PUBLIC thirdparty)
target_link_libraries(eval_eca PUBLIC landscape_opt)

add_executable(test exec/test.cpp)
target_include_directories(test PUBLIC include)
target_include_directories(test PUBLIC thirdparty)
target_link_libraries(test PUBLIC landscape_opt)

add_executable(biorevaix_test exec/biorevaix_test.cpp)
target_include_directories(biorevaix_test PUBLIC include)
target_include_directories(biorevaix_test PUBLIC thirdparty)
target_link_libraries(biorevaix_test PUBLIC landscape_opt)

add_executable(contraction_test test/contraction_test.cpp)
target_include_directories(contraction_test PUBLIC include)
target_include_directories(contraction_test PUBLIC thirdparty)
target_link_libraries(contraction_test PUBLIC landscape_opt)

add_executable(eca_contribution_analysis exec/eca_contribution_analysis.cpp)
target_include_directories(eca_contribution_analysis PUBLIC include)
target_include_directories(eca_contribution_analysis PUBLIC thirdparty)
target_link_libraries(eca_contribution_analysis PUBLIC landscape_opt)


