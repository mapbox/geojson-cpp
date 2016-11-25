CXXFLAGS += -I include -std=c++14 -Wall -Wextra -Wshadow -Werror -O3 -fPIC

MASON ?= .mason/mason
VARIANT = variant 1.1.4
GEOMETRY = geometry 0.9.0
RAPIDJSON = rapidjson 1.1.0

DEPS = `$(MASON) cflags $(VARIANT)` `$(MASON) cflags $(GEOMETRY)`
RAPIDJSON_DEP = `$(MASON) cflags $(RAPIDJSON)`

default: build/libgeojson.a

mason_packages/headers/geometry:
	$(MASON) install $(VARIANT)
	$(MASON) install $(GEOMETRY)
	$(MASON) install $(RAPIDJSON)

build:
	mkdir -p build

CFLAGS += -fvisibility=hidden

build/geojson.o: src/mapbox/geojson.cpp include/mapbox/geojson.hpp include/mapbox/geojson_impl.hpp build mason_packages/headers/geometry Makefile
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(DEPS) $(RAPIDJSON_DEP) -c $< -o $@

build/libgeojson.a: build/geojson.o
	$(AR) -rcs $@ $<

build/test: test/test.cpp test/fixtures/* build/libgeojson.a
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(DEPS) $(RAPIDJSON_DEP) $< -Lbuild -lgeojson -o $@

test: build/test
	./build/test

format:
	clang-format include/mapbox/*.hpp src/mapbox/geojson.cpp test/*.cpp -i

clean:
	rm -rf build
