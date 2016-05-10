CFLAGS += -I include --std=c++14 -Wall -Wextra -Werror -O3 -fPIC

export MASON_DIR = $(shell pwd)/.mason
export MASON = $(MASON_DIR)/mason

VARIANT = variant 1.1.0
GEOMETRY = geometry 0.4.0
RAPIDJSON = rapidjson 1.0.2

DEPS = `$(MASON) cflags $(VARIANT)` `$(MASON) cflags $(GEOMETRY)`
RAPIDJSON_DEP = `$(MASON) cflags $(RAPIDJSON)`

default:
	make run-test

$(MASON_DIR):
	git submodule update --init $(MASON_DIR)

mason_packages: $(MASON_DIR)
	$(MASON) install $(VARIANT)
	$(MASON) install $(GEOMETRY)
	$(MASON) install $(RAPIDJSON)

build:
	mkdir -p build

build/geojson.o: src/mapbox/geojson.cpp include/mapbox/geojson.hpp build mason_packages Makefile
	$(CXX) $(CFLAGS) $(DEPS) $(RAPIDJSON_DEP) -c $< -o $@

build/libgeojson.a: build/geojson.o
	$(AR) -rcs $@ $<

build/test: test/test.cpp test/fixtures/* build/libgeojson.a
	$(CXX) $(CFLAGS) $(DEPS) $< -Lbuild -lgeojson -o $@

run-test: build/test
	./build/test

format:
	clang-format include/mapbox/*.hpp src/mapbox/geojson.cpp test/*.cpp -i

clean:
	rm -rf build
