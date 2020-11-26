CC=g++-9
CC_NORM=c++2a

CPUS ?= $(shell getconf _NPROCESSORS_ONLN || echo 1)
MAKEFLAGS += --jobs=$(CPUS)

INCLUDE_DIR=include
THIRDPARTY_DIR=thirdparty
SRC_DIR=src
BUILD_DIR=build
EXEC_SUBDIR=exec

CATCH2_INCLUDE_DIR=~/Libs/Catch2/
EIGEN_INCLUDE_DIR=~/Libs/eigen-eigen-323c052e1731/
COINOR_INCLUDE_DIR=~/Libs/coinor/dist/include/
COINOR_LIB_PATH=~/Libs/coinor/dist/lib/
LEMON_INCLUDE_DIR=~/Libs/lemon-1.3.1/
GUROBI_INCLUDE_PATH="$(GUROBI_HOME)/include/"
GUROBI_LIB_PATH="$(GUROBI_HOME)/lib/"

INCLUDE_PATHS=$(INCLUDE_DIR) $(THIRDPARTY_DIR) $(SRC_DIR) $(EIGEN_INCLUDE_DIR) $(COINOR_INCLUDE_DIR) $(LEMON_INCLUDE_DIR) $(GUROBI_INCLUDE_PATH)
LIBS_PATHS=$(COINOR_LIB_PATH) $(GUROBI_LIB_PATH)

INCLUDE_FLAGS=$(foreach d, $(INCLUDE_PATHS), -I $d)
LIBS_FLAGS=$(foreach d, $(LIBS_PATHS), -L $d) -lCbc -lCbcSolver -lClp -lOsiClp -lOsiCbc -lCoinUtils -lCgl -lemon -lgurobi_c++ -lgurobi90 -lOsiGrb -pthread -ltbb

#-DNDEBUG -O2
CFLAGS=-g -W -Wall -Wno-deprecated-copy -ansi -pedantic -std=$(CC_NORM) -fconcepts -O2 -flto -march=native -pipe $(INCLUDE_FLAGS)
LDFLAGS=$(LIBS_FLAGS)
LSFLAGS=-static $(LIBS_FLAGS) -lmpi

EXEC=solve solver_analysis
EXEC_EXTENSION=.out

CPP_PATHS:=$(shell find $(SRC_DIR) -name '*.cpp')
CPPSRC_PATHS:=$(filter-out $(SRC_DIR)/$(EXEC_SUBDIR)/%,$(CPP_PATHS))
CPPEXEC_PATHS:=$(filter $(SRC_DIR)/$(EXEC_SUBDIR)/%,$(CPP_PATHS))

BUILD_SUBDIRS:=$(sort $(filter-out ./,$(dir $(CPP_PATHS:$(SRC_DIR)/%=$(BUILD_DIR)/%))))

build_dir:
	mkdir -p $(BUILD_DIR) $(BUILD_SUBDIRS)

OBJ=$(CPPSRC_PATHS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

objs: build_dir $(OBJ)

all : objs $(EXEC)


solve: objs $(BUILD_DIR)/$(EXEC_SUBDIR)/solve.o
	$(CC) -o $@$(EXEC_EXTENSION) $(OBJ) $(BUILD_DIR)/$(EXEC_SUBDIR)/solve.o $(LDFLAGS)

contraction_test: objs $(BUILD_DIR)/$(EXEC_SUBDIR)/contraction_test.o
	$(CC) -o $@$(EXEC_EXTENSION) $(OBJ) $(BUILD_DIR)/$(EXEC_SUBDIR)/contraction_test.o $(LDFLAGS)

solver_analysis: objs $(BUILD_DIR)/$(EXEC_SUBDIR)/solver_analysis.o
	$(CC) -o $@$(EXEC_EXTENSION) $(OBJ) $(BUILD_DIR)/$(EXEC_SUBDIR)/solver_analysis.o $(LDFLAGS)


clean:
	rm -rf $(BUILD_DIR)/*
mrproper: clean
	rmdir $(BUILD_DIR)
	rm -f *$(EXEC_EXTENSION)

documentation:
	doxywizard $$PWD/doc/Doxyfile
	xdg-open doc/html/index.html