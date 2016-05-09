#include <mapbox/geojson.hpp>
#include <mapbox/geometry.hpp>

#include <cassert>
#include <fstream>
#include <sstream>

using namespace mapbox::geojson;

geojson readGeoJSON(const std::string &path) {
    std::ifstream t(path.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    return parse(buffer.str());
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

static void testGeometryCollection() {
    const auto &data = readGeoJSON("test/fixtures/geometry-collection.json");
    assert(data.is<geometry>());

    const auto &geom = data.get<geometry>();
    assert(geom.is<geometry_collection>());

    const auto &collection = geom.get<geometry_collection>();
    assert(collection[0].is<point>());
    assert(collection[1].is<line_string>());
}

static void testFeature() {
    const auto &data = readGeoJSON("test/fixtures/feature.json");
    assert(data.is<feature>());

    const auto &f = data.get<feature>();
    assert(f.geometry.is<point>());

    assert(f.properties.at("bool").is<bool>());
    assert(f.properties.at("bool") == true);
    assert(f.properties.at("string").is<std::string>());
    assert(f.properties.at("string").get<std::string>() == "foo");
    assert(f.properties.at("double") == 2.5);
    assert(f.properties.at("double").is<double>());
    assert(f.properties.at("uint").get<std::uint64_t>() == 10);
    assert(f.properties.at("uint").is<std::uint64_t>());
    assert(f.properties.at("int").get<std::int64_t>() == -10);
    assert(f.properties.at("int").is<std::int64_t>());

    // not implemented
    assert(f.properties.at("null") == false);
    assert(f.properties.at("nested") == false);
}

static void testFeatureCollection() {
    const auto &data = readGeoJSON("test/fixtures/feature-collection.json");
    assert(data.is<feature_collection>());

    const auto &features = data.get<feature_collection>();
    assert(features.size() == 2);
}

int main() {
    testPoint();
    testMultiPoint();
    testLineString();
    testMultiLineString();
    testPolygon();
    testMultiPolygon();
    testGeometryCollection();
    testFeature();
    testFeatureCollection();
    return 0;
}
