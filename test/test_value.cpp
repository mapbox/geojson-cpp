#include <mapbox/geojson.hpp>
#include <mapbox/geojson/rapidjson.hpp>
#include <mapbox/geojson/value.hpp>
#include <mapbox/geometry.hpp>

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace mapbox::geojson;

std::string toString(const rapidjson_value &jsvalue) {
    assert(jsvalue.IsString());
    return { jsvalue.GetString(), jsvalue.GetStringLength() };
}

mapbox::geojson::value toValue(const rapidjson_value &jsvalue) {
    if (jsvalue.IsNull()) {
        return {};
    }

    if (jsvalue.IsArray()) {
        mapbox::geojson::value::array_type values;
        values.reserve(jsvalue.GetArray().Size());
        for (const auto &v : jsvalue.GetArray()) {
            values.emplace_back(toValue(v));
        }
        return values;
    }

    if (jsvalue.IsObject()) {
        mapbox::geojson::value::object_type valueMap;
        for (const auto &pair : jsvalue.GetObject()) {
            valueMap.emplace(toString(pair.name), toValue(pair.value));
        }
        return valueMap;
    }

    if (jsvalue.IsString()) {
        return toString(jsvalue);
    }

    if (jsvalue.IsBool()) {
        return jsvalue.GetType() == rapidjson::kTrueType ? true : false;
    }

    if (jsvalue.IsNumber()) {
        if (jsvalue.IsUint64())
            return std::uint64_t(jsvalue.GetUint64());
        if (jsvalue.IsInt64())
            return std::int64_t(jsvalue.GetInt64());
        return jsvalue.GetDouble();
    }

    assert(false);
    return {};
}

template <typename Expected = geometry>
void test(const std::string &path, bool use_convert = false) {
    std::ifstream t(path.c_str());
    std::stringstream buffer;
    buffer << t.rdbuf();
    const auto &json = buffer.str();
    rapidjson_document d;
    d.Parse<0>(json.c_str());
    if (d.HasParseError()) {
        throw std::runtime_error("Can't parse geojson document");
    }

    const mapbox::geojson::value convertedValue = toValue(d);
    const geojson result                        = convert(convertedValue);
    const mapbox::geojson::value roundTripValue = convert(result);
    const geojson expected                      = use_convert ? convert<geometry>(d) : parse(json);
    const geojson resultFromStringValue         = convert(value{ json });
    const geojson roundTrip                     = convert(roundTripValue);

    assert(expected == result);
    assert(expected == resultFromStringValue);
    assert(expected == roundTrip);
    assert(result.is<Expected>());
}

int main() {
    test("test/fixtures/null.json", true);
    test("test/fixtures/point.json");
    test("test/fixtures/multi-point.json");
    test("test/fixtures/line-string.json");
    test("test/fixtures/multi-line-string.json");
    test("test/fixtures/polygon.json");
    test("test/fixtures/multi-polygon.json");
    test("test/fixtures/geometry-collection.json");
    test<feature>("test/fixtures/feature.json");
    test<feature>("test/fixtures/feature-null-properties.json");
    test<feature>("test/fixtures/feature-missing-properties.json");
    test<feature_collection>("test/fixtures/feature-collection.json");
    test<feature_collection>("test/fixtures/feature-id.json");
    try {
        test("test/fixtures/array.json");
    } catch (const std::runtime_error &err) {
        assert(std::string(err.what()).find("Invalid") != std::string::npos);
    }
    return 0;
}
