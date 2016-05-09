CFLAGS += -I include --std=c++14 -Wall -Wextra -Werror -O3

export MASON_DIR = $(shell pwd)/.mason
export MASON = $(MASON_DIR)/mason

VARIANT = variant 1.1.0
GEOMETRY = geometry 0.3.0
RAPIDJSON = rapidjson 1.0.2

DEPS = `$(MASON) cflags $(VARIANT)` `$(MASON) cflags $(GEOMETRY)` `$(MASON) cflags $(RAPIDJSON)`

default:
	make run-test

$(MASON_DIR):
	git submodule update --init $(MASON_DIR)

mason_packages: $(MASON_DIR)
	$(MASON) install $(VARIANT)
	$(MASON) install $(GEOMETRY)
	$(MASON) install $(RAPIDJSON)

build/test: test/test.cpp test/fixtures/* include/mapbox/* mason_packages Makefile
	mkdir -p build
	$(CXX) test/test.cpp $(CFLAGS) $(DEPS) $(RAPIDJSON_DEP) -o build/test

run-test: build/test
	./build/test

format:
	clang-format include/mapbox/*.hpp test/*.cpp -i

clean:
	rm -rf build
