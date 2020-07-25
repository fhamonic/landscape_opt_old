requires base packages build-essential git gcc-9 g++-9 litbb-dev
and cmake gfortran for compiling the thirdparties

requires Eigen, Clp, Cbc, Lemon:

from package manager :
    sudo apt install libeigen3-dev
    sudo apt install coinor-libclp-dev coinor-libcbc-dev
    sudo apt install liblemon-dev
    
I recommend getting librairies from sources for lattest versions : 

OpenBLAS:
    mkdir OpenBLAS
    git clone https://github.com/xianyi/OpenBLAS.git
# edit Makefile.rules: CC = gcc ; FC = gfortran ; COMMON_OPT = -O2 -march=native
    make all
    mkdir ../build
    make PREFIX=/home/plaiseek/Libs/OpenBlas/dist install

Csdp:
    cd coinor
    mkdir Csdp
    git clone https://github.com/coin-or/Csdp.git
# edit Makefile.rules: CC = gcc ; FC = gfortran ; COMMON_OPT = -O2 -march=native
    make all


Eigen : headers only, just needs to be downloaded : http://eigen.tuxfamily.org/index.php?title=Main_Page

Clp, Cbc, Dip : using coinbrew :
    mkdir coinor
    cd coinor
    git clone https://github.com/coin-or/coinbrew
    export OPT_CXXFLAGS="-pipe -flto -O3 -march=native"
    export OPT_CFLAGS="-pipe -flto -O3 -march=native"
    export LDFLAGS="-pipe -flto"
    ./coinbrew/coinbrew build Clp:releases/1.17.5 --enable-static
    ./coinbrew/coinbrew build Cbc:releases/2.10.5 --enable-cbc-parallel --enable-static
    ./coinbrew/coinbrew build Dip --enable-cbc-parallel --enable-static
    add 'export LD_LIBRARY_PATH=/home/plaiseek/Libs/coinor/dist/lib' to your ~/.bashrc
        
Csdp:
    cd coinor
    mkdir Csdp
    git clone https://github.com/coin-or/Csdp.git
# edit Makefile.rules: replace /usr/local by ../dist modify BLAS linking to -L../../../OpenBLAS/lib -lopenblas
    make CC=gcc all

Lemon : http://lemon.cs.elte.hu/trac/lemon/wiki/Downloads
    cd lemon-x.y.z
    mkdir build
    cd build
    cmake ..
    make
    sudo make install

Then specify the paths in the makefile