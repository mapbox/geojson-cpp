CFLAGS += -I include --std=c++14 -Wall -Wextra -Werror -O3 -fPIC

MASON = .mason/mason
VARIANT = variant 1.1.0
GEOMETRY = geometry 0.4.0
RAPIDJSON = rapidjson 1.0.2

DEPS = `$(MASON) cflags $(VARIANT)` `$(MASON) cflags $(GEOMETRY)`
RAPIDJSON_DEP = `$(MASON) cflags $(RAPIDJSON)`

default: build/libgeojson.a

mason_packages:
	git submodule update --init .mason
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

test: build/test
	./build/test

format:
	clang-format include/mapbox/*.hpp src/mapbox/geojson.cpp test/*.cpp -i

clean:
	rm -rf build
