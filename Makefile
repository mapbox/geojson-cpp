CXXFLAGS += -Iinclude -std=c++14 -Wall -Wextra -Wshadow -Werror -O3

MASON ?= .mason/mason
VARIANT = variant 1.1.0
GEOMETRY = geometry 0.8.0
RAPIDJSON = rapidjson 1.1.0

DEPS = `$(MASON) cflags $(VARIANT)` `$(MASON) cflags $(GEOMETRY)`
RAPIDJSON_DEP = `$(MASON) cflags $(RAPIDJSON)`

default: mason_packages/headers/geometry build build/test

mason_packages/headers/geometry:
	$(MASON) install $(VARIANT)
	$(MASON) install $(GEOMETRY)
	$(MASON) install $(RAPIDJSON)

build:
	mkdir -p build

CFLAGS += -fvisibility=hidden

build/test:
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(DEPS) $(RAPIDJSON_DEP) test/test.cpp -o ./build/test

test: mason_packages/headers/geometry build build/test
	./build/test

format:
	clang-format include/mapbox/*.hpp test/*.cpp -i

clean:
	rm -rf build
