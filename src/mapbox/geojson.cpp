#include <mapbox/geojson.hpp>
#include <rapidjson/document.h>

namespace mapbox {
namespace geojson {

using json_value = rapidjson::Value;

template <typename T>
T convert(const json_value &json);

template <>
point convert<point>(const json_value &json) {
    if (json.Size() < 2)
        throw error("coordinates array must have at least 2 numbers");

    return point{ json[0].GetDouble(), json[1].GetDouble() };
}

template <typename Cont>
Cont convert(const json_value &json) {
    Cont points;
    auto size = json.Size();
    points.reserve(size);

    for (rapidjson::SizeType i = 0; i < size; ++i) {
        points.push_back(convert<typename Cont::value_type>(json[i]));
    }
    return points;
}

template <>
geometry convert<geometry>(const json_value &json) {
    if (!json.IsObject())
        throw error("Geometry must be an object");

    if (!json.HasMember("type"))
        throw error("Geometry must have a type property");

    const auto &type = json["type"];

    if (type == "GeometryCollection") {
        if (!json.HasMember("geometries"))
            throw error("GeometryCollection must have a geometries property");

        const auto &json_geometries = json["geometries"];

        if (!json_geometries.IsArray())
            throw error("GeometryCollection geometries property must be an array");

        return geometry{ convert<geometry_collection>(json_geometries) };
    }

    if (!json.HasMember("coordinates"))
        throw error(std::string(type.GetString()) + " geometry must have a coordinates property");

    const auto &json_coords = json["coordinates"];
    if (!json_coords.IsArray())
        throw error("coordinates property must be an array");

    if (type == "Point")
        return geometry{ convert<point>(json_coords) };
    if (type == "MultiPoint")
        return geometry{ convert<multi_point>(json_coords) };
    if (type == "LineString")
        return geometry{ convert<line_string>(json_coords) };
    if (type == "MultiLineString")
        return geometry{ convert<multi_line_string>(json_coords) };
    if (type == "Polygon")
        return geometry{ convert<polygon>(json_coords) };
    if (type == "MultiPolygon")
        return geometry{ convert<multi_polygon>(json_coords) };

    throw error(std::string(type.GetString()) + " not yet implemented");
}

template <>
value convert<value>(const json_value &json) {
    if (json.IsBool())
        return value{ json.GetBool() };
    if (json.IsDouble())
        return value{ json.GetDouble() };
    if (json.IsUint())
        return value{ std::uint64_t(json.GetUint()) };
    if (json.IsInt())
        return value{  std::int64_t(json.GetInt()) };
    if (json.IsString())
        return value{ std::string(json.GetString()) };

    // TODO json.IsArray()
    // TODO json.IsObject()
    // TODO json.IsNull()
    return false;
}

template <>
feature convert<feature>(const json_value &json) {
    if (!json.IsObject())
        throw error("Feature must be an object");
    if (!json.HasMember("type"))
        throw error("Feature must have a type property");
    if (json["type"] != "Feature")
        throw error("Feature type must be Feature");
    if (!json.HasMember("geometry"))
        throw error("Feature must have a geometry property");

    const auto &json_geometry = json["geometry"];

    if (!json_geometry.IsObject())
        throw error("Feature geometry must be an object");

    if (!json.HasMember("properties"))
        throw error("Feature must have a properties property");

    feature feature{ convert<geometry>(json_geometry) };

    const auto &json_props = json["properties"];

    if (json_props.IsNull())
        return feature;

    for (auto itr = json_props.MemberBegin(); itr != json_props.MemberEnd(); ++itr) {
        feature.properties[itr->name.GetString()] = convert<value>(itr->value);
    }

    return feature;
}

template <>
geojson convert<geojson>(const json_value &json) {
    if (!json.IsObject())
        throw error("GeoJSON must be an object");
    if (!json.HasMember("type"))
        throw error("GeoJSON must have a type property");

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
            collection.push_back(convert<feature>(json_features[i]));
        }

        return geojson{ collection };
    }

    if (type == "Feature")
        return geojson{ convert<feature>(json) };

    return geojson{ convert<geometry>(json) };
}

template <class T>
T parse(const std::string &json) {
    rapidjson::Document d;
    d.Parse(json.c_str());
    return convert<T>(d);
}

template <> geometry parse<geometry>(const std::string &);
template <> feature parse<feature>(const std::string &);
template <> feature_collection parse<feature_collection>(const std::string &);

geojson parse(const std::string &json) {
    return parse<geojson>(json);
}

}
}
