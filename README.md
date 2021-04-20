## Dependencies

### Build process
The build process requires CMake 3.12 (https://cmake.org/) or more and the Conan C++ package manager (https://conan.io/).
#### Ubuntu
    sudo apt install cmake
    pip install conan
#### Manjaro
    sudo pamac install cmake
    pip install conan

### Libraries
The project uses COINOR libraries Clp Cbc and LEMON that are not currently available in Conan so get the binaries from your system packet manage or compile them (except LEMON that is header only) from sources

#### From package manager
##### Ubuntu
    sudo apt install coinor-libclp-dev coinor-libcbc-dev
    sudo apt install liblemon-dev
##### Manjaro (AUR)
    sudo pamac install coin-or-lemon

### From sources
#### Lemon : http://lemon.cs.elte.hu/trac/lemon/wiki/Downloads
    cd lemon-x.y.z
    mkdir build
    cd build
    cmake ..
    make
    sudo make install

#### Gurobi : https://www.gurobi.com/
    linux64/bin/grbgetkey <licence_key>
    cd linux64/src/build
    make
    mv libgurobi_c++.a ../../lib

add to .bashrc:

    export GUROBI_HOME="/home/plaiseek/Libs/gurobi911/linux64"
    export PATH="$PATH:$GUROBI_HOME/bin"
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$GUROBI_HOME/lib"

#### Compile Clp and Cbc using coinbrew :
##### Requirements
###### Ubuntu
    sudo apt install build-essential git gcc-9 g++-9 cmake gfortran
###### Manjaro
    pamac install base-devel git cmake gcc-fortran

##### Instructions
    mkdir coinor
    cd coinor
    git clone https://github.com/coin-or/coinbrew
<!-- export OPT_CFLAGS="-pipe -flto -march=native"
    export OPT_CXXFLAGS="-pipe -flto -march=native"
    export LDFLAGS="-L$GUROBI_HOME/lib/ -pipe -flto" -->
    export LDFLAGS="-L$GUROBI_HOME/lib/"
    ./coinbrew/coinbrew fetch Cbc:releases/2.10.5
    ./coinbrew/coinbrew build Cbc:releases/2.10.5 --enable-cbc-parallel --with-gurobi-incdir="$GUROBI_HOME/include/" --with-gurobi-lib="-L$GUROBI_HOME/lib/ -lm -lpthread -lgurobi_c++ -lgurobi91" ADD_FFLAGS=-fallow-argument-mismatch

add to .bashrc :

    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/plaiseek/Libs/coinor/dist/lib/"

## How to Compile
Just type :

    make