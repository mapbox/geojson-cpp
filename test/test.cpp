#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <mapbox/geojson.hpp>
#include <mapbox/geometry.hpp>

#include <cassert>
#include <cstdio>
#include <iostream>

using namespace mapbox::geojson;

geojson readGeoJSON(const std::string &path) {
    std::FILE *fp = std::fopen(path.c_str(), "r");
    char buffer[65536];
    rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
    rapidjson::Document d;
    d.ParseStream(is);
    return convert(d);
}

static void testPoint() {
    const auto &data = readGeoJSON("test/fixtures/point.json");
    assert(data.is<geometry>());

    const auto &geom = data.get<geometry>();
    assert(geom.is<point>());

    const auto &p = geom.get<point>();
    assert(p.x == 30.5);
    assert(p.y == 50.5);
}

static void testMultiPoint() {
    const auto &data = readGeoJSON("test/fixtures/multi-point.json");
    assert(data.is<geometry>());

    const auto &geom = data.get<geometry>();
    assert(geom.is<multi_point>());

    const auto &points = geom.get<multi_point>();
    assert(points.size() == 2);
}

int main() {
    testPoint();
    testMultiPoint();
    return 0;
}
