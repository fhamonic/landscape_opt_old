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
