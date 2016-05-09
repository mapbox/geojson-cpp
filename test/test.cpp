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

static void testLineString() {
    const auto &data = readGeoJSON("test/fixtures/line-string.json");
    assert(data.is<geometry>());

    const auto &geom = data.get<geometry>();
    assert(geom.is<line_string>());

    const auto &points = geom.get<line_string>();
    assert(points.size() == 2);
}

static void testMultiLineString() {
    const auto &data = readGeoJSON("test/fixtures/multi-line-string.json");
    assert(data.is<geometry>());

    const auto &geom = data.get<geometry>();
    assert(geom.is<multi_line_string>());

    const auto &lines = geom.get<multi_line_string>();
    assert(lines.size() == 1);
    assert(lines[0].size() == 2);
}

static void testPolygon() {
    const auto &data = readGeoJSON("test/fixtures/polygon.json");
    assert(data.is<geometry>());

    const auto &geom = data.get<geometry>();
    assert(geom.is<polygon>());

    const auto &rings = geom.get<polygon>();
    assert(rings.size() == 1);
    assert(rings[0].size() == 5);
    assert(rings[0][0] == rings[0][4]);
}

static void testMultiPolygon() {
    const auto &data = readGeoJSON("test/fixtures/multi-polygon.json");
    assert(data.is<geometry>());

    const auto &geom = data.get<geometry>();
    assert(geom.is<multi_polygon>());

    const auto &polygons = geom.get<multi_polygon>();
    assert(polygons.size() == 1);
    assert(polygons[0].size() == 1);
    assert(polygons[0][0].size() == 5);
    assert(polygons[0][0][0] == polygons[0][0][4]);
}

int main() {
    testPoint();
    testMultiPoint();
    testLineString();
    testMultiLineString();
    testPolygon();
    testMultiPolygon();
    return 0;
}
