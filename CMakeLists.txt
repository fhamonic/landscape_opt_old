cmake_minimum_required(VERSION 3.12)

project(Landscape_Opt VERSION 1.0
    DESCRIPTION "Library for handling ecological landscape datas and optimize their connectivity according to the PC indicator .. and more incomming"
    LANGUAGES CXX)

# set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

#################### Options ####################
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CONAN_DISABLE_CHECK_COMPILER ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeBuildTypeUtils.cmake)

option(WARNINGS "" OFF)
option(COMPILE_FOR_NATIVE "" OFF)
option(COMPILE_WITH_LTO "" OFF)
option(WITH_GUROBI "" OFF)

#################### Packages ####################
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(LEMON REQUIRED)
find_package(COIN REQUIRED)
if(${WITH_GUROBI} STREQUAL "ON")
    find_package(GUROBI REQUIRED)
endif()

##################### Sources ######################
file(GLOB SRC "src/*/*.cpp" "src/*/*/*.cpp" "src/*/*/*/*.cpp" "src/helper.cpp") 

#################### LIB target ####################
add_library(landscape_opt ${SRC})

target_include_directories(landscape_opt PUBLIC include)
target_include_directories(landscape_opt PUBLIC thirdparty)

target_link_libraries(landscape_opt CONAN_PKG::fast-cpp-csv-parser)
target_link_libraries(landscape_opt CONAN_PKG::tbb)
target_link_libraries(landscape_opt CONAN_PKG::boost)
target_link_libraries(landscape_opt CONAN_PKG::eigen)
target_link_libraries(landscape_opt CONAN_PKG::range-v3)

target_link_libraries(landscape_opt ${LEMON_LIBRARIES})
target_link_libraries(landscape_opt ${COIN_LIBRARIES})

if(${WITH_GUROBI} STREQUAL "ON")
    target_include_directories(landscape_opt PUBLIC ${GUROBI_INCLUDE_DIR})
    target_link_libraries(landscape_opt ${GUROBI_LIBRARIES})
endif()

target_compile_options_if_exists(landscape_opt PRIVATE ${WARNINGS} -Wall)
target_compile_options_if_exists(landscape_opt PRIVATE ${COMPILE_FOR_NATIVE} -march=native)
target_compile_options_if_exists(landscape_opt PRIVATE ${COMPILE_WITH_LTO} -flto)

################### EXEC target ####################
add_executable(solve exec/solve.cpp)
target_include_directories(solve PUBLIC include)
target_include_directories(solve PUBLIC thirdparty)
target_link_libraries(solve PUBLIC landscape_opt)




add_executable(eval_eca exec/eval_eca.cpp)
target_include_directories(eval_eca PUBLIC include)
target_include_directories(eval_eca PUBLIC thirdparty)
target_link_libraries(eval_eca PUBLIC landscape_opt)

add_executable(test exec/test.cpp)
target_include_directories(test PUBLIC include)
target_include_directories(test PUBLIC thirdparty)
target_link_libraries(test PUBLIC landscape_opt)
