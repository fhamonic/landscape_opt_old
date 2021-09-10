# Landscape_Opt

Library for modeling and optimizing ecological landscapes according to the PC connectivity indicator.

[![Generic badge](https://img.shields.io/badge/C++-17-blue.svg?style=flat&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/17)
[![Generic badge](https://img.shields.io/badge/CMake-3.12+-blue.svg?style=flat&logo=cmake)](https://cmake.org/cmake/help/latest/release/3.12.html)
[![Generic badge](https://img.shields.io/badge/Conan--blue.svg?style=flat&logo=data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iVVRGLTgiIHN0YW5kYWxvbmU9Im5vIj8+CjxzdmcKICAgdmVyc2lvbj0iMS4xIgogICB3aWR0aD0iNDgxcHgiCiAgIGhlaWdodD0iNTEycHgiCiAgIGlkPSJzdmc0IgogICBzb2RpcG9kaTpkb2NuYW1lPSJjb25hbl9pY29uXzEzMjQ4MC5zdmciCiAgIGlua3NjYXBlOnZlcnNpb249IjEuMSAoYzRlOGY5ZWQ3NCwgMjAyMS0wNS0yNCkiCiAgIHhtbG5zOmlua3NjYXBlPSJodHRwOi8vd3d3Lmlua3NjYXBlLm9yZy9uYW1lc3BhY2VzL2lua3NjYXBlIgogICB4bWxuczpzb2RpcG9kaT0iaHR0cDovL3NvZGlwb2RpLnNvdXJjZWZvcmdlLm5ldC9EVEQvc29kaXBvZGktMC5kdGQiCiAgIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIKICAgeG1sbnM6c3ZnPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyI+CiAgPGRlZnMKICAgICBpZD0iZGVmczgiIC8+CiAgPHNvZGlwb2RpOm5hbWVkdmlldwogICAgIGlkPSJuYW1lZHZpZXc2IgogICAgIHBhZ2Vjb2xvcj0iI2ZmZmZmZiIKICAgICBib3JkZXJjb2xvcj0iIzY2NjY2NiIKICAgICBib3JkZXJvcGFjaXR5PSIxLjAiCiAgICAgaW5rc2NhcGU6cGFnZXNoYWRvdz0iMiIKICAgICBpbmtzY2FwZTpwYWdlb3BhY2l0eT0iMC4wIgogICAgIGlua3NjYXBlOnBhZ2VjaGVja2VyYm9hcmQ9IjAiCiAgICAgc2hvd2dyaWQ9ImZhbHNlIgogICAgIGlua3NjYXBlOnpvb209IjEuNDE5OTIxOSIKICAgICBpbmtzY2FwZTpjeD0iMjQwLjUwNjE5IgogICAgIGlua3NjYXBlOmN5PSIyNTMuNTM1MDgiCiAgICAgaW5rc2NhcGU6d2luZG93LXdpZHRoPSIxOTIwIgogICAgIGlua3NjYXBlOndpbmRvdy1oZWlnaHQ9IjEwMTUiCiAgICAgaW5rc2NhcGU6d2luZG93LXg9IjEyMDAiCiAgICAgaW5rc2NhcGU6d2luZG93LXk9Ijg3OSIKICAgICBpbmtzY2FwZTp3aW5kb3ctbWF4aW1pemVkPSIxIgogICAgIGlua3NjYXBlOmN1cnJlbnQtbGF5ZXI9InN2ZzQiIC8+CiAgPHBhdGgKICAgICBkPSJtMjIyLjY5Mjc4IDIwNy42NzY3NmMtNDYuNzg4MDUtMi45MDA2MS0xNTEuMDEzMDYtNjIuOTY3MTUtNDkuNTk5MzQtMTIxLjA3MTgyIDgzLjQyNTkzLTQ3Ljc5ODYxOSAxNjAuODAyMDUtMTQuMjgyMTczIDIwOC40MDgwNiAzNi43OTE3NDIgMCAwLTI3LjM2OTgzIDE2Ljg1MjU0LTU5LjIwNSAzMi4wNDAzNiAyMi40OTQxMy01MC4yODU5NS02MS4zMDA3OS03OC4zNzM3ODQtMTA5LjM1NTE0LTUyLjIzOTcxLTQ4LjA1NDM0IDI2LjEzNDA3LTUyLjc3MzY5IDc5Ljc0MTcxIDY1LjQ3Mzc4IDc2LjYxODI1em0yNTcuODI1MTYgMTQ2LjcwNDI4di0yMzUuMDI0MjdsLTI0NS4xMzY3Mi0xMTkuMzU2NzctMjM1LjM4MTIyIDExMy4yOTg5NnYyMjYuNzExMzFsMjQyLjcyNjYzIDE3MS45ODk3M3ptLTI0NS4yNDE3MS0zNDIuNDAxOSAyMzAuNDkyOTggMTE1Ljc4NzE4LTIzMC4xNDY1MSAxMzEuMjMwOTMtMjIzLjA4NzEyLTEzNy45NDU5N3ptMjMwLjQ5MDkzIDEyOS44ODI4M3YyMDIuNTkzNTRsLTkxLjg2MzA1IDU5Ljc0MDg1di0yMDguNjA3MjZ6bS0xMDYuOTYzNTEgNjMuNDgxMDd2MjA5LjI2NjU3bC0xMTYuMTA4OTYgNzMuMjUxMjR2LTIxNi41MjEyNXoiCiAgICAgaWQ9InBhdGgyIiAvPgogIDxwYXRoCiAgICAgc3R5bGU9ImZpbGw6I2ZmZmZmZjtzdHJva2Utd2lkdGg6MC43MDQyNjQiCiAgICAgZD0iTSAxMjEuNzQ1MzMsNDI1LjcyMzYgMC43MDQyNjQxLDMzOS45NjExOCBWIDIyNi42OTkyMyAxMTMuNDM3MjggTCAxMDYuMzQzODgsNjIuNTc1MTM4IEMgMTY0LjQ0NTY3LDM0LjYwMDk2MSAyMTcuMjY2MzgsOS4xODc5ODAxIDIyMy43MjMyNSw2LjEwMTg0NjggTCAyMzUuNDYzLDAuNDkwNjk1MTcgMzE0LjIyMTE4LDM4Ljg0MTUxNyBjIDQzLjMxNywyMS4wOTI5NTIgOTguMzI3OTMsNDcuODg2MjMzIDEyMi4yNDY1LDU5LjU0MDYyOCBsIDQzLjQ4ODMsMjEuMTg5ODA1IC0wLjAwNCwxMTcuMzM2NDUgLTAuMDA0LDExNy4zMzY0NCAtNjYuMzczNTUsNDMuOTk2NSBjIC0zNi41MDU0NSwyNC4xOTgwNyAtODkuODY3MTEsNTkuNTc3MzQgLTExOC41ODE0Niw3OC42MjA1OSBsIC01Mi4yMDc1MywzNC42MjQxIHogbSAxODIuMTQ0NjMsMjQuMDc0MDggNTQuNTgwNDcsLTM0LjQ0ODAyIDAuMTc4MzcsLTEwNS4xMDQ1NyBjIDAuMTQyNTUsLTg0LjAwMDg0IDAuMDAxLC0xMDUuMDM1NzQgLTAuNzA0MjcsLTEwNC43NjE3OSAtMC40ODU0NSwwLjE4ODUyIC0yNi41NTMwNiwxNC45NDExNCAtNTcuOTI4MDIsMzIuNzgzNiBsIC01Ny4wNDU0LDMyLjQ0MDgyIC0wLjE3ODI4LDEwOC44NDQ1OCAtMC4xNzgyOSwxMDguODQ0NTYgMy4zNDc0OCwtMi4wNzU1OCBjIDEuODQxMTEsLTEuMTQxNTcgMjcuOTA4NjgsLTE3LjU3NzE5IDU3LjkyNzk0LC0zNi41MjM2IHogbSAxMTguNTYyMDYsLTc2LjUzNzcxIDQzLjM2MTE0LC0yOC4xNzA1NiAwLjAyODgsLTEwMS42NjU0NSBjIDAuMDI1OSwtOTEuNDkzNCAtMC4wODAzLC0xMDEuNjIzNTggLTEuMDYxNTQsLTEwMS4yNDcwNSAtMC41OTk2OCwwLjIzMDEyIC0yMS4zNDM5MiwxMi4yMjc3MSAtNDYuMDk4MzEsMjYuNjYxMzIgbCAtNDUuMDA3OTksMjYuMjQyOTMgLTAuMDMxLDEwNC44MzA3OSAtMC4wMzEsMTA0LjgzMDggMi43MzkzOCwtMS42NTYxMSBjIDEuNTA2NjMsLTAuOTEwODYgMjIuMjUxODYsLTE0LjMzMjg2IDQ2LjEwMDQ5LC0yOS44MjY2NyB6IE0gMzUwLjcyNDcsMTkzLjg3MTM0IGMgNjMuMzMwMywtMzYuMTA3NDggMTE1LjE0NiwtNjUuODUwMTQgMTE1LjE0NiwtNjYuMDk0NzkgMCwtMC4yNDQ2NCAtNTEuODcyNjcsLTI2LjUwOTQ5IC0xMTUuMjcyNjEsLTU4LjM2NjMyNiBMIDIzNS4zMjU0OSwxMS40ODg3MDIgMjA4LjUxMjgxLDI0LjYxOTgxMyBjIC0xNC43NDY5Nyw3LjIyMjExMSAtNjQuODAyNjcsMzEuNzMzNzE0IC0xMTEuMjM0ODk1LDU0LjQ3MDIyOCAtNDYuNDMyMjIsMjIuNzM2NTE5IC04NC40NjI0ODEsNDEuNjEwNTM5IC04NC41MTE2OSw0MS45NDIyNjkgLTAuMDYzMjQsMC40MjYzNiAyMjEuMDY1NDc1LDEzNy45MjEwMSAyMjIuNjM1MjI1LDEzOC40MzEzOCAwLjA5NzUsMC4wMzE3IDUxLjk5Mjk1LC0yOS40ODQ4NiAxMTUuMzIzMjUsLTY1LjU5MjM1IHoiCiAgICAgaWQ9InBhdGg4OTkiIC8+CiAgPHBhdGgKICAgICBzdHlsZT0iZmlsbDojZmZmZmZmO3N0cm9rZS13aWR0aDowLjcwNDI2NCIKICAgICBkPSJtIDIxNC44MDA1NSwyMDYuMzEwNDMgYyAtMjQuMTIxNTksLTQuMDc1NzcgLTUxLjEzNjMxLC0xNy44MjcyOSAtNjcuNjA5MzUsLTM0LjQxNTczIC0xMS4xMzk3OCwtMTEuMjE3ODEgLTE1LjYzMDEyLC0xOS43NDQ4MSAtMTYuMzQ2NzMsLTMxLjA0MTg4IC0xLjI4MjA5LC0yMC4yMTE4NSAxNi40MzU3NiwtNDAuNjk4OTEgNDkuOTk2ODEsLTU3LjgxMTAzOSAyMS45NzI4LC0xMS4yMDM1MDYgNDEuMDUyMTMsLTE2Ljc4NjYwMyA2My45MTAyLC0xOC43MDE3NTQgNDQuMTYyMiwtMy43MDAxMDQgOTMuODM0NzMsMTYuNDk5MjAzIDEyOS44MDk4NCw1Mi43ODcwOTMgbCA2LjIzMTE5LDYuMjg1MzcgLTkuNzUyNTEsNS42Mjc1MiBjIC0xOC4wMDc4OSwxMC4zOTExNSAtNDYuNDkzMjEsMjUuMzQwNzUgLTQ3LjA3ODczLDI0LjcwNzc0IC0wLjExMDk4LC0wLjEyIDAuMzgsLTIuNDM2NjEgMS4wOTEwOCwtNS4xNDgwMyA1LjAzNDAzLC0xOS4xOTUzMiAtNS4wNTk4OSwtMzYuMjMxOTYgLTI3Ljc3MzA5LC00Ni44NzU3MSAtMTIuMTYxLC01LjY5ODg0NiAtMjYuMjM1NTUsLTguNTczMTc1IC00MS45ODM1MiwtOC41NzM5NTUgLTE2LjMzOTY3LC04LjA4ZS00IC0yOC43MTYyNSwyLjc5MzE4OCAtNDEuNTUxNTksOS4zODAyMTUgLTE3Ljg5Mzg1LDkuMTgzMDMgLTI5Ljk5ODM5LDIyLjYzMDg1IC0zMi40NzY2MSwzNi4wODA2NyAtMS42ODg3OCw5LjE2NTMyIDIuNTAxNjYsMTguNjk3MDEgMTAuOTk2NDksMjUuMDEyOTggMTQuMjUyOTUsMTAuNTk3MTcgMzkuMDc0MywxNi4yNTU5NiA3MS43MzM1NiwxNi4zNTM4NyBsIDEyLjc3ODIsMC4wMzgzIC0yNy4xMTQxNywxMy42NTY1NCBjIC0xNC45MTI3OSw3LjUxMTA5IC0yNy41ODk1NCwxMy42NDE1MSAtMjguMTcwNTYsMTMuNjIzMTYgLTAuNTgxMDIsLTAuMDE4NCAtMy41OTE3NSwtMC40NjE3NyAtNi42OTA1MSwtMC45ODUzNiB6IgogICAgIGlkPSJwYXRoOTM4IiAvPgo8L3N2Zz4K)](https://cmake.org/cmake/help/latest/release/3.12.html)

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
