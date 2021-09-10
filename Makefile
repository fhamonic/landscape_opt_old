MAKEFLAGS += --no-print-directory

CPUS?=$(shell getconf _NPROCESSORS_ONLN || echo 1)

BUILD_DIR = build

.PHONY: all clean doc

all: $(BUILD_DIR)
	@cd $(BUILD_DIR) && \
	cmake --build . --parallel $(CPUS)

$(BUILD_DIR):
	@mkdir $(BUILD_DIR) && \
	cd $(BUILD_DIR) && \
	conan install .. && \
	cmake -DCMAKE_CXX_COMPILER=g++-10 -DCMAKE_BUILD_TYPE=Release -DWARNINGS=ON -DHARDCORE_WARNINGS=OFF -DCOMPILE_FOR_NATIVE=OFF -DCOMPILE_WITH_LTO=OFF -DWITH_GUROBI=ON ..

clean:
	@rm -rf $(BUILD_DIR)

doc:
	doxywizard $$PWD/docs/Doxyfile
	xdg-open docs/html/index.html 