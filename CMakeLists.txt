cmake_minimum_required(VERSION 3.12)

project(Landscape_Opt VERSION 1.0
    DESCRIPTION "Library for handling ecological landscape datas and optimize their connectivity according to the PC indicator .. and more incomming"
    LANGUAGES CXX)

# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

#################### Packages ####################
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(LEMON REQUIRED)
find_package(COIN REQUIRED)


#################### Options ####################
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeBuildTypeUtils.cmake)

option(WARNINGS "" OFF)
option(COMPILE_FOR_NATIVE "" OFF)
option(COMPILE_WITH_LTO "" OFF)



##################### Sources ######################
file(GLOB SRC "src/*/*.cpp") 

#################### LIB target ####################
add_library(landscape_opt ${SRC})

target_include_directories(landscape_opt PUBLIC include)
target_include_directories(landscape_opt PUBLIC thirdparty)

target_link_libraries(landscape_opt CONAN_PKG::tbb)
target_link_libraries(landscape_opt CONAN_PKG::boost)
target_link_libraries(landscape_opt CONAN_PKG::eigen)

target_link_libraries(landscape_opt COIN)
target_link_libraries(landscape_opt LEMON)

target_compile_options_if_exists(landscape_opt PRIVATE ${WARNINGS} -Wall)
target_compile_options_if_exists(landscape_opt PRIVATE ${COMPILE_FOR_NATIVE} -march=native)
target_compile_options_if_exists(landscape_opt PRIVATE ${COMPILE_WITH_LTO} -flto)


################### EXEC target ####################
add_executable(solve exec/solve.cpp)

target_include_directories(solve PUBLIC include)
target_include_directories(solve PUBLIC thirdparty)

target_link_libraries(solve PUBLIC landscape_opt)
