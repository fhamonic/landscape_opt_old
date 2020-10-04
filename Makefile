CC=g++-9
CC_NORM=c++2a

CPUS ?= $(shell getconf _NPROCESSORS_ONLN || echo 1)
MAKEFLAGS += --jobs=$(CPUS)

SRC_DIR=src
BUILD_DIR=build

CATCH2_INCLUDE_DIR=~/Libs/Catch2/
EIGEN_INCLUDE_DIR=~/Libs/eigen-eigen-323c052e1731/
COINOR_INCLUDE_DIR=~/Libs/coinor/dist/include/
COINOR_LIB_PATH=~/Libs/coinor/dist/lib/
LEMON_INCLUDE_DIR=~/Libs/lemon-1.3.1/
GUROBI_INCLUDE_PATH=~/Libs/gurobi901/linux64/include/
GUROBI_LIB_PATH=~/Libs/gurobi901/linux64/lib/


INCLUDE_FLAGS=-I include -I thirdparty -I $(SRC_DIR) -I $(EIGEN_INCLUDE_DIR) -I $(COINOR_INCLUDE_DIR) -I $(LEMON_INCLUDE_DIR) -I $(GUROBI_INCLUDE_PATH)

#-DNDEBUG
CFLAGS=-g -W -Wall -Wno-deprecated-copy -ansi -pedantic -std=$(CC_NORM) -fconcepts -O2 -flto -march=native -pipe $(INCLUDE_FLAGS) -L $(COINOR_LIB_PATH)
LDFLAGS=-L $(COINOR_LIB_PATH) -lCbc -lClp -lOsiClp -lCoinUtils -lemon -L $(GUROBI_LIB_PATH) -lgurobi_c++ -lgurobi90 -lOsiGrb -pthread -ltbb
LSFLAGS=-static -L $(COINOR_LIB_PATH) -lCbc -lClp -lOsiClp -lCoinUtils -lemon -L $(GUROBI_LIB_PATH) -lgurobi_c++ -lgurobi90 -lOsiGrb -pthread -lmpi -ltbb

EXEC=pl_markov pl_eca glutton_eca
EXTENSION=.out

LANDSCAPE_SRC:=landscape/landscape.cpp landscape/decored_landscape.cpp
INDICES_SRC:=indices/eca.cpp
PLANS_SRC:=solvers/concept/restoration_option.cpp solvers/concept/restoration_plan.cpp
NAIVE_SOLVERS_SRC:=solvers/naive_eca_dec.cpp solvers/naive_eca_inc.cpp solvers/bogo.cpp
GLUTTON_SOLVERS_SRC:=solvers/glutton_eca_inc.cpp solvers/glutton_eca_dec.cpp
PL_SOLVERS_SRC:=solvers/pl_eca_2.cpp solvers/pl_eca_3.cpp solvers/randomized_rounding.cpp
PARSERS_SRC:=parsers/std_landscape_parser.cpp parsers/std_restoration_plan_parser.cpp
PRECOMPUTATION_SRC:=precomputation/concept/contraction_precomputation.cpp precomputation/my_contraction_algorithm.cpp

SRC:=$(LANDSCAPE_SRC) $(INDICES_SRC) $(PLANS_SRC) $(NAIVE_SOLVERS_SRC) $(GLUTTON_SOLVERS_SRC) $(PL_SOLVERS_SRC) $(PARSERS_SRC) $(PRECOMPUTATION_SRC) helper.cpp osi_builder.cpp

OBJ=$(addprefix $(BUILD_DIR)/,$(SRC:.cpp=.o))

build_dir: 
	mkdir -p $(BUILD_DIR) $(BUILD_DIR)/landscape $(BUILD_DIR)/indices $(BUILD_DIR)/solvers $(BUILD_DIR)/solvers/concept $(BUILD_DIR)/parsers $(BUILD_DIR)/precomputation $(BUILD_DIR)/precomputation/concept
output_dir: 
	mkdir -p output
dir : build_dir output_dir

all : dir $(OBJ) $(EXEC)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -o $@ -c $< $(CFLAGS)


	
pl_reff: $(OBJ) $(BUILD_DIR)/pl_reff.o
	$(CC) -o $@$(EXTENSION) $^ $(LDFLAGS)

pl_max_independent: $(OBJ) $(BUILD_DIR)/pl_max_independent.o
	$(CC) -o $@$(EXTENSION) $^ $(LDFLAGS)

solve: $(OBJ) $(BUILD_DIR)/solve.o
	$(CC) -o $@$(EXTENSION) $^ $(LDFLAGS)


test: $(OBJ) $(BUILD_DIR)/test.o
	$(CC) -o $@$(EXTENSION) $^ $(LDFLAGS)

contraction_test: $(OBJ) $(BUILD_DIR)/contraction_test.o
	$(CC) -o $@$(EXTENSION) $^ $(LDFLAGS)

pl_test: $(OBJ) $(BUILD_DIR)/pl_test.o
	$(CC) -o $@$(EXTENSION) $^ $(LDFLAGS)

contracted_benefits: $(OBJ) $(BUILD_DIR)/contracted_benefits.o
	$(CC) -o $@$(EXTENSION) $^ $(LDFLAGS)

generate_instance: $(OBJ) $(BUILD_DIR)/generate_instance.o
	$(CC) -o $@$(EXTENSION) $^ $(LDFLAGS)

generate_marseille: $(OBJ) $(BUILD_DIR)/generate_marseille.o
	$(CC) -o $@$(EXTENSION) $^ $(LDFLAGS)
	
convert_multiplicative: $(OBJ) $(BUILD_DIR)/convert_multiplicative.o
	$(CC) -o $@$(EXTENSION) $^ $(LDFLAGS)

eca_analysis: $(OBJ) $(BUILD_DIR)/eca_analysis.o
	$(CC) -o $@$(EXTENSION) $^ $(LDFLAGS)

solver_analysis: $(OBJ) $(BUILD_DIR)/solver_analysis.o
	$(CC) -o $@$(EXTENSION) $^ $(LDFLAGS)


random_chooser_test: $(BUILD_DIR)/random_chooser_test.o
	$(CC) -o $@$(EXTENSION) $^ $(LDFLAGS)

# test: $(OBJ)
# 	$(CC) -o $(BUILD_DIR)/test.o -c $(SRC_DIR)/test.cpp -g -W -Wall -std=$(CC_NORM) -I $(INCLUDE_DIR) -fopenmp
# 	$(CC) -o $@$(EXTENSION) $(BUILD_DIR)/test.o -fopenmp


clean:
	rm -rf $(BUILD_DIR)/*.o
	rm -rf $(BUILD_DIR)/**/*.o
	rm -rf $(BUILD_DIR)/**/**/*.o

mrproper: clean
	rm -rf $(BUILD_DIR)
	rm -rf -f *$(EXTENSION)

documentation:
	doxywizard $$PWD/doc/Doxyfile
	xdg-open doc/html/index.html