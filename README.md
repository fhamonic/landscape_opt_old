## Dependencies

### From package manager

#### Ubuntu

    sudo apt install build-essential git gcc-9 g++-9 litbb2 libtbb-dev cmake gfortran

requires Eigen, Clp, Cbc, Lemon:

    sudo apt install libeigen3-dev
    sudo apt install coinor-libclp-dev coinor-libcbc-dev
    sudo apt install liblemon-dev

#### Manjaro

    pamac install base-devel git cmake gcc-fortran
    
### From sources (recommended)


#### Lemon : http://lemon.cs.elte.hu/trac/lemon/wiki/Downloads

    cd lemon-x.y.z
    mkdir build
    cd build
    cmake ..
    make
    sudo make install

#### Eigen : headers only, just needs to be downloaded : http://eigen.tuxfamily.org/index.php?title=Main_Page

#### Gurobi : https://www.gurobi.com/

    linux64/bin/grbgetkey <licence_key>
    cd linux64/src/build
    make
    mv libgurobi_c++.a ../../lib

add to .bashrc:

    export GUROBI_HOME="/home/plaiseek/Libs/gurobi911/linux64"
    export PATH="$PATH:$GUROBI_HOME/bin"
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$GUROBI_HOME/lib"

#### Clp, Cbc : using coinbrew :

    mkdir coinor
    cd coinor
    git clone https://github.com/coin-or/coinbrew
<!-- export OPT_CFLAGS="-pipe -flto -march=native"
    export OPT_CXXFLAGS="-pipe -flto -march=native"
    export LDFLAGS="-L$GUROBI_HOME/lib/ -pipe -flto" -->
    export LDFLAGS="-L$GUROBI_HOME/lib/"
     
    ./coinbrew/coinbrew fetch Cbc:releases/2.10.5
    ./coinbrew/coinbrew build Cbc:releases/2.10.5 --enable-cbc-parallel --with-gurobi-incdir="$GUROBI_HOME/include/" --with-gurobi-lib="-L$GUROBI_HOME/lib/ -lm -lpthread -lgurobi_c++ -lgurobi91" ADD_FFLAGS=-fallow-argument-mismatch
<!-- 
-->

add to .bashrc :

    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/home/plaiseek/Libs/coinor/dist/lib/"

      
#### OpenBLAS:

    mkdir OpenBLAS
    git clone https://github.com/xianyi/OpenBLAS.git
<!--- edit Makefile.rules: CC = gcc ; FC = gfortran ; COMMON_OPT = -O2 -march=native -->
    make all
    mkdir ../build
    make PREFIX=/home/plaiseek/Libs/OpenBlas/dist install

#### Csdp:
    cd coinor
    mkdir Csdp
    git clone https://github.com/coin-or/Csdp.git
<!---  edit Makefile.rules: CC = gcc ; FC = gfortran ; COMMON_OPT = -O2 -march=native -->
<!---  edit Makefile.rules: replace /usr/local by ../dist modify BLAS linking to -L../../../OpenBLAS/lib -lopenblas -->
    make CC=gcc all


Then specify the paths in the makefile
