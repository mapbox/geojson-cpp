#pragma once

#include <mapbox/geometry.hpp>
#include <mapbox/variant.hpp>
#include <rapidjson/document.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

namespace mapbox {
namespace geojson {

struct geojson_empty {};

using line_string = mapbox::geometry::line_string<double>;
using multi_point = mapbox::geometry::multi_point<double>;
using point = mapbox::geometry::point<double>;
using geometry = mapbox::geometry::geometry<double>;
using feature = mapbox::geometry::feature<double>;
using feature_collection = mapbox::geometry::feature_collection<double>;

using geojson = mapbox::util::variant<geojson_empty, geometry, feature, feature_collection>;

using json_value = rapidjson::Value;
using error = std::runtime_error;

point convertPoint(const json_value &json) {
    if (json.Size() < 2) throw error("coordinates array must have at least 2 numbers");
    return point{ json[0].GetDouble(), json[1].GetDouble() };
}

template <typename T>
geometry convertPoints(const json_value &json) {
    T points;
    auto size = json.Size();
    points.reserve(size);

    for (rapidjson::SizeType i = 0; i < size; ++i) {
        const auto &p = convertPoint(json);
        points.push_back(p);
    }
    return geometry{ points };
}

geometry convertGeometry(const json_value &json) {
    if (!json.IsObject()) throw error("Geometry must be an object");
    if (!json.HasMember("type")) throw error("Geometry must have a type property");

    const auto &type = json["type"];

    if (type == "GeometryCollection") {
        if (!json.HasMember("geometries"))
            throw error("GeometryCollection must have a geometries property");

        const auto &json_geometries = json["geometries"];

        if (!json_geometries.IsArray())
            throw error("GeometryCollection geometries property must be an array");

        throw error("GeometryCollection not yet implemented");
    }

    if (!json.HasMember("coordinates"))
        throw std::runtime_error("GeoJSON geometry must have a coordinates property");

    const auto &json_coords = json["coordinates"];
    if (!json_coords.IsArray()) throw error("coordinates property must be an array");

    if (type == "Point") {
        return geometry{ convertPoint(json_coords) };
    }
    if (type == "MultiPoint") {
        return convertPoints<multi_point>(json_coords);
    }
    if (type == "LineString") {
        return convertPoints<line_string>(json_coords);
    }

    throw error(std::string(type.GetString()) + " not yet implemented");
}

feature convertFeature(const json_value &json) {
    if (!json.IsObject()) throw error("Feature must be an object");
    if (!json.HasMember("type")) throw error("Feature must have a type property");
    if (json["type"] != "Feature") throw error("Feature type must be Feature");
    if (!json.HasMember("geometry")) throw error("Feature must have a geometry property");

    const auto &json_geometry = json["geometry"];

    if (!json_geometry.IsObject()) throw error("Feature geometry must be an object");

    const auto &geometry = convertGeometry(json_geometry);

    feature feature{ geometry };
    // TODO feature properties

    return feature;
}

geojson convert(const json_value &json) {
    if (!json.IsObject()) throw error("GeoJSON must be an object");
    if (!json.HasMember("type")) throw error("GeoJSON must have a type property");

    const auto &type = json["type"];

    if (type == "FeatureCollection") {
        if (!json.HasMember("features"))
            throw error("FeatureCollection must have features property");

        const auto &json_features = json["features"];

        if (!json_features.IsArray())
            throw error("FeatureCollection features property must be an array");

        feature_collection collection;

        const auto &size = json_features.Size();
        collection.reserve(size);

        for (rapidjson::SizeType i = 0; i < size; ++i) {
            const auto &feature = convertFeature(json_features[i]);
            collection.push_back(feature);
        }

        return geojson{ collection };
    }

    throw error(std::string(type.GetString()) + " not yet implemented");
}

} // namespace geojson
} // namespace mapbox
