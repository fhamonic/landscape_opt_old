# Landscape_Opt

Library for modeling and optimizing ecological landscapes according to the PC connectivity indicator.

[![Generic badge](https://img.shields.io/badge/C++-17-blue.svg?style=flat&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/17)
[![Generic badge](https://img.shields.io/badge/CMake-3.12+-blue.svg?style=flat&logo=cmake)](https://cmake.org/cmake/help/latest/release/3.12.html)

[![Generic badge](https://img.shields.io/badge/license-Boost%20Software%20License-blue)](https://www.boost.org/users/license.html)

## Dependencies

### Build process
The build process requires CMake 3.12 (https://cmake.org/) or more and the Conan C++ package manager (https://conan.io/).
#### Ubuntu
    sudo apt install cmake
    pip install conan
#### Manjaro
    sudo pamac install cmake
    pip install conan

##### Configure Conan for GCC >= 5.1
    conan profile update settings.compiler.libcxx=libstdc++11 default

### Libraries
The project uses COIN-OR libraries such as LEMON for implementing graphs and Cbc as a default Mixed Integer Programming solver. libraries Clp Cbc and LEMON that are not currently available in Conan so get the binaries from your system packet manager or compile them (except LEMON that is header only) from sources

#### From package manager
##### Ubuntu
    sudo apt install liblemon-dev
    sudo apt install coinor-libclp-dev coinor-libcbc-dev
##### Manjaro (AUR)
    sudo pamac install coin-or-lemon
    sudo apt install coin-or-cbc

#### From sources
##### Lemon : http://lemon.cs.elte.hu/trac/lemon/wiki/Downloads
    cd lemon-x.y.z
    mkdir build
    cd build
    cmake ..
    make
    sudo make install
    
pass -DCMAKE_INSTALL_PREFIX=<install path> to cmake to specify the install directory, default is /user/local

##### Cbc using coinbrew :
###### Ubuntu requirements
    sudo apt install build-essential git gcc-9 g++-9 cmake gfortran
###### Manjaro requirements
    pamac install base-devel git cmake gcc-fortran

###### Instructions
    mkdir coinor
    cd coinor
    git clone https://github.com/coin-or/coinbrew
    ./coinbrew/coinbrew fetch Cbc:releases/2.10.5
    ./coinbrew/coinbrew build Cbc:releases/2.10.5 --enable-cbc-parallel

add to .bashrc :

    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:<path_to>/coinor/dist/lib/"

##### Gurobi : https://www.gurobi.com/
    linux64/bin/grbgetkey <licence_key>
    cd linux64/src/build
    make
    mv libgurobi_c++.a ../../lib

add to .bashrc:

    export GUROBI_HOME="<path_to>/linux64"
    export PATH="$PATH:$GUROBI_HOME/bin"
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$GUROBI_HOME/lib"


## How to Compile
    make

## Usage
making the project will produce a static library "llandscape_opt.a" and an executable "solve"

    solve <landscape_file> <problem_file> <budget_value> <solver_name> [<option>=<value>]

A wrong call of "solve" will output the available solvers names if the provided one doesn't exist and available the available options for the selected solver otherwise.

"<landscape_file>" is the path to a csv file with columns "patches_file" and "links_file" giving paths to the csv files describing the set of patches and of links of the landscape.

"<problem_file>" is the path to a file describing the available options to improve elements of the landscape.

See the data repertory for examples.

## Acknowledgments
This work is part of the PhD thesis of François Hamonic which is funded by Region Sud (https://www.maregionsud.fr/) and Natural Solutions (https://www.natural-solutions.eu/)
