CXXFLAGS += -I include -std=c++14 -Wall -Wextra -Wshadow -Werror -O3 -fPIC

MASON ?= .mason/mason
VARIANT = variant 1.1.4
GEOMETRY = geometry 1.1.0
RAPIDJSON = rapidjson 1.1.0

DEPS = `$(MASON) cflags $(VARIANT)` `$(MASON) cflags $(GEOMETRY)`
RAPIDJSON_DEP = `$(MASON) cflags $(RAPIDJSON)`

default: build/libgeojson.so

mason_packages/headers/geometry:
	$(MASON) install $(VARIANT)
	$(MASON) install $(GEOMETRY)
	$(MASON) install $(RAPIDJSON)

build:
	mkdir -p build

CFLAGS += -fvisibility=hidden

build/geojson.o: src/mapbox/geojson.cpp include/mapbox/geojson.hpp include/mapbox/geojson_impl.hpp include/mapbox/geojson_value_impl.hpp build mason_packages/headers/geometry Makefile
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(DEPS) $(RAPIDJSON_DEP) -c $< -o $@

build/libgeojson.so: build/geojson.o
	$(CXX) -shared $< -o $@

build/test: test/test.cpp test/fixtures/* build/libgeojson.so
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(DEPS) $(RAPIDJSON_DEP) $< -Lbuild -lgeojson -o $@

build/test_value: test/test_value.cpp test/fixtures/* build/libgeojson.so
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(DEPS) $(RAPIDJSON_DEP) $< -Lbuild -lgeojson -o $@

test: build/test build/test_value
	LD_LIBRARY_PATH=`pwd`/build ./build/test
	LD_LIBRARY_PATH=`pwd`/build ./build/test_value

format:
	clang-format include/mapbox/*.hpp src/mapbox/geojson.cpp test/*.cpp -i

clean:
	rm -rf build
