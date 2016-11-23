#include <mapbox/geojson.hpp>
#include <mapbox/geometry.hpp>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace mapbox::geojson;
using namespace mapbox::geometry;

geojson<double> readGeoJSON(const std::string &path, bool use_convert) {
    std::ifstream t(path.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    if (use_convert) {
        rapidjson_document d;
        d.Parse<0>(buffer.str().c_str());
        return convert_geojson<double>(d);
    } else {
        return parse(buffer.str());
    }
}

template <class T>
std::string writeGeoJSON(const T &t, bool use_convert) {
    if (use_convert) {
        rapidjson_allocator allocator;
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        convert(t, allocator).Accept(writer);
        return buffer.GetString();
    } else {
        return stringify(t);
    }
}

static void testPoint(bool use_convert) {
    const auto &data = readGeoJSON("test/fixtures/point.json", use_convert);
    assert(data.is<geometry<double>>());

    const auto &geom = data.get<geometry<double>>();
    assert(geom.is<point<double>>());

    const auto &p = geom.get<point<double>>();
    assert(p.x == 30.5);
    assert(p.y == 50.5);

    auto out = parse(writeGeoJSON(data, use_convert));
    assert(out == data);
}

static void testMultiPoint(bool use_convert) {
    const auto &data = readGeoJSON("test/fixtures/multi-point.json", use_convert);
    assert(data.is<geometry<double>>());

    const auto &geom = data.get<geometry<double>>();
    assert(geom.is<multi_point<double>>());

    const auto &points = geom.get<multi_point<double>>();
    assert(points.size() == 2);

    auto out = parse(writeGeoJSON(data, use_convert));
    assert(out == data);
}

static void testLineString(bool use_convert) {
    const auto &data = readGeoJSON("test/fixtures/line-string.json", use_convert);
    assert(data.is<geometry<double>>());

    const auto &geom = data.get<geometry<double>>();
    assert(geom.is<line_string<double>>());

    const auto &points = geom.get<line_string<double>>();
    assert(points.size() == 2);

    auto out = parse(writeGeoJSON(data, use_convert));
    assert(out == data);
}

static void testMultiLineString(bool use_convert) {
    const auto &data = readGeoJSON("test/fixtures/multi-line-string.json", use_convert);
    assert(data.is<geometry<double>>());

    const auto &geom = data.get<geometry<double>>();
    assert(geom.is<multi_line_string<double>>());

    const auto &lines = geom.get<multi_line_string<double>>();
    assert(lines.size() == 1);
    assert(lines[0].size() == 2);

    auto out = parse(writeGeoJSON(data, use_convert));
    assert(out == data);
}

static void testPolygon(bool use_convert) {
    const auto &data = readGeoJSON("test/fixtures/polygon.json", use_convert);
    assert(data.is<geometry<double>>());

    const auto &geom = data.get<geometry<double>>();
    assert(geom.is<polygon<double>>());

    const auto &rings = geom.get<polygon<double>>();
    assert(rings.size() == 1);
    assert(rings[0].size() == 5);
    assert(rings[0][0] == rings[0][4]);

    auto out = parse(writeGeoJSON(data, use_convert));
    assert(out == data);
}

static void testMultiPolygon(bool use_convert) {
    const auto &data = readGeoJSON("test/fixtures/multi-polygon.json", use_convert);
    assert(data.is<geometry<double>>());

    const auto &geom = data.get<geometry<double>>();
    assert(geom.is<multi_polygon<double>>());

    const auto &polygons = geom.get<multi_polygon<double>>();
    assert(polygons.size() == 1);
    assert(polygons[0].size() == 1);
    assert(polygons[0][0].size() == 5);
    assert(polygons[0][0][0] == polygons[0][0][4]);

    auto out = parse(writeGeoJSON(data, use_convert));
    assert(out == data);
}

static void testGeometryCollection(bool use_convert) {
    const auto &data = readGeoJSON("test/fixtures/geometry-collection.json", use_convert);
    assert(data.is<geometry<double>>());

    const auto &geom = data.get<geometry<double>>();
    assert(geom.is<geometry_collection<double>>());

    const auto &collection = geom.get<geometry_collection<double>>();
    assert(collection[0].is<point<double>>());
    assert(collection[1].is<line_string<double>>());

    auto out = parse(writeGeoJSON(data, use_convert));
    assert(out == data);
}

static void testFeature(bool use_convert) {
    const auto &data = readGeoJSON("test/fixtures/feature.json", use_convert);
    assert(data.is<feature<double>>());

    const auto &f = data.get<feature<double>>();
    assert(f.geometry.is<point<double>>());

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
    assert(f.properties.at("null").is<mapbox::geometry::null_value_t>());
    assert(f.properties.at("null") == nullptr);

    using prop_map = std::unordered_map<std::string, value>;
    using values   = std::vector<value>;

    const auto &nested = f.properties.at("nested");

    // Explicit recursive_wrapper as a workaround for https://github.com/mapbox/variant/issues/102
    assert(nested.is<mapbox::util::recursive_wrapper<values>>());
    assert(nested.get<values>().at(0).is<std::uint64_t>());
    assert(nested.get<values>().at(0).get<std::uint64_t>() == 5);
    assert(nested.get<values>().at(1).is<mapbox::util::recursive_wrapper<prop_map>>());
    assert(nested.get<values>().at(1).get<prop_map>().at("foo").is<std::string>());
    assert(nested.get<values>().at(1).get<prop_map>().at("foo").get<std::string>() == "bar");

    auto out = parse(writeGeoJSON(data, use_convert));
    assert(out == data);
}

static void testFeatureNullProperties(bool use_convert) {
    const auto &data = readGeoJSON("test/fixtures/feature-null-properties.json", use_convert);
    assert(data.is<feature<double>>());

    const auto &f = data.get<feature<double>>();
    assert(f.geometry.is<point<double>>());
    assert(f.properties.size() == 0);

    auto out = parse(writeGeoJSON(data, use_convert));
    assert(out == data);
}

static void testFeatureCollection(bool use_convert) {
    const auto &data = readGeoJSON("test/fixtures/feature-collection.json", use_convert);
    assert(data.is<feature_collection<double>>());

    const auto &features = data.get<feature_collection<double>>();
    assert(features.size() == 2);

    auto out = parse(writeGeoJSON(data, use_convert));
    assert(out == data);
}

static void testFeatureID(bool use_convert) {
    const auto &data = readGeoJSON("test/fixtures/feature-id.json", use_convert);
    assert(data.is<feature_collection<double>>());

    const auto &features = data.get<feature_collection<double>>();

    assert(features.at(0).id == identifier{ 1234 });
    assert(features.at(1).id == identifier{ "abcd" });

    auto out = parse(writeGeoJSON(data, use_convert));
    assert(out == data);
}

void testAll(bool use_convert) {
    testPoint(use_convert);
    testMultiPoint(use_convert);
    testLineString(use_convert);
    testMultiLineString(use_convert);
    testPolygon(use_convert);
    testMultiPolygon(use_convert);
    testGeometryCollection(use_convert);
    testFeature(use_convert);
    testFeatureNullProperties(use_convert);
    testFeatureCollection(use_convert);
    testFeatureID(use_convert);
}

int main() {
    testAll(true);
    testAll(false);
    return 0;
}
