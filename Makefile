CC=g++-9
CC_NORM=c++2a

CPUS?=$(shell getconf _NPROCESSORS_ONLN || echo 1)
MAKEFLAGS+=--jobs=$(CPUS)

INCLUDE_DIR=include
THIRDPARTY_DIR=thirdparty
SRC_DIR=src
BUILD_DIR=build
EXEC_SUBDIR=exec
EXEC_EXTENSION=out

##################################################
################## Depedencies ###################
##################################################

CATCH2_INCLUDE_DIR=~/Libs/Catch2/
EIGEN_INCLUDE_DIR=~/Libs/eigen-3.3.9/
COINOR_INCLUDE_DIR=~/Libs/coinor/dist/include/
LEMON_INCLUDE_DIR=~/Libs/lemon-1.3.1/
GUROBI_INCLUDE_PATH="$(GUROBI_HOME)/include/"
#CTRE_INCLUDE_PATH=~/Libs/compile-time-regular-expressions/single-header/

COINOR_LIB_PATH=~/Libs/coinor/dist/lib/
GUROBI_LIB_PATH="$(GUROBI_HOME)/lib/"

INCLUDE_PATHS=$(INCLUDE_DIR) $(THIRDPARTY_DIR) $(SRC_DIR) $(EIGEN_INCLUDE_DIR) $(COINOR_INCLUDE_DIR) $(LEMON_INCLUDE_DIR) $(GUROBI_INCLUDE_PATH) #$(CTRE_INCLUDE_PATH)
LIBS_PATHS=$(COINOR_LIB_PATH) $(GUROBI_LIB_PATH)

##################################################
######### Compilation and Linking flags ##########
##################################################

INCLUDE_FLAGS=$(foreach d, $(INCLUDE_PATHS), -I $d)
LIBS_FLAGS=$(foreach d, $(LIBS_PATHS), -L $d)

CFLAGS=-W -Wall -Wno-deprecated-copy -ansi -pedantic -std=$(CC_NORM) -fconcepts -O2 -flto -march=native -pipe $(INCLUDE_FLAGS)
CFLAGS_DEBUG=-g -W -Wall -Wno-deprecated-copy -ansi -pedantic -std=$(CC_NORM) -fconcepts -flto -march=native -pipe $(INCLUDE_FLAGS)

LDFLAGS=$(LIBS_FLAGS) -lCbc -lCbcSolver -lClp -lOsiClp -lOsiCbc -lCoinUtils -lCgl -lemon -lgurobi_c++ -lgurobi90 -lOsiGrb -pthread -ltbb -Wl,--as-needed

LSFLAGS=$(LIBS_FLAGS) -Wl,-Bstatic -pthread -lgurobi_c++ -lemon
LSFLAGS:=$(LSFLAGS) -Wl,-Bdynamic -ltbb -lmpi -lgurobi91 -lCbc -lCbcSolver -lClp -lOsiClp -lOsiCbc -lOsiGrb -lCoinUtils -lCgl -Wl,--as-needed

##################################################
########## Source files and Build paths ##########
##################################################

CPP_PATHS:=$(shell find $(SRC_DIR) -name '*.cpp')
CPPSRC_PATHS:=$(filter-out $(SRC_DIR)/$(EXEC_SUBDIR)/%,$(CPP_PATHS))
CPPEXEC_PATHS:=$(filter $(SRC_DIR)/$(EXEC_SUBDIR)/%,$(CPP_PATHS))

EXEC=$(CPPEXEC_PATHS:$(SRC_DIR)/$(EXEC_SUBDIR)/%.cpp=%)

BUILD_SUBDIRS:=$(sort $(filter-out ./,$(dir $(CPP_PATHS:$(SRC_DIR)/%=$(BUILD_DIR)/%))))

OBJ=$(CPPSRC_PATHS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
OBJ_DEBUG=$(CPPSRC_PATHS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.og)

##################################################
############### Compilation rules ################
##################################################

build_dir:
	mkdir -p $(BUILD_DIR) $(BUILD_SUBDIRS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

$(BUILD_DIR)/%.og: $(SRC_DIR)/%.cpp
	$(CC) -o $@ -c $< $(CFLAGS_DEBUG)

objs: build_dir $(OBJ)
objs-debug: build_dir $(OBJ_DEBUG)

$(foreach _exec, $(EXEC), $(_exec)): objs
	$(eval _exec:=$@)
	$(CC) -o $(BUILD_DIR)/$(EXEC_SUBDIR)/$(_exec).o -c $(SRC_DIR)/$(EXEC_SUBDIR)/$(_exec).cpp $(CFLAGS)
	$(CC) -o $(_exec).$(EXEC_EXTENSION) $(OBJ) $(BUILD_DIR)/$(EXEC_SUBDIR)/$(_exec).o $(LDFLAGS)

$(foreach _exec, $(EXEC), $(_exec)-debug): objs-debug
	$(eval _exec:=$(patsubst %-debug,%,$@))
	$(CC) -o $(BUILD_DIR)/$(EXEC_SUBDIR)/$(_exec).og -c $(SRC_DIR)/$(EXEC_SUBDIR)/$(_exec).cpp $(CFLAGS_DEBUG)
	$(CC) -o $@.$(EXEC_EXTENSION) $(OBJ_DEBUG) $(BUILD_DIR)/$(EXEC_SUBDIR)/$(_exec).og $(LDFLAGS)

$(foreach _exec, $(EXEC), $(_exec)-static): objs
	$(eval _exec:=$(patsubst %-static,%,$@))
	$(CC) -o $(BUILD_DIR)/$(EXEC_SUBDIR)/$(_exec).o -c $(SRC_DIR)/$(EXEC_SUBDIR)/$(_exec).cpp $(CFLAGS)
	$(CC) -o $@.$(EXEC_EXTENSION) $(OBJ) $(BUILD_DIR)/$(EXEC_SUBDIR)/$(_exec).o $(LSFLAGS)

##################################################
################# General rules ##################
##################################################

clean:
	rm -rf $(BUILD_DIR)/*
	
mrproper: clean
	rm -df $(BUILD_DIR)
	rm -f *.$(EXEC_EXTENSION)

documentation:
	doxywizard $$PWD/doc/Doxyfile
	xdg-open doc/html/index.html