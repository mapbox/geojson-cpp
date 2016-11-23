#pragma once

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <mapbox/geometry.hpp>
#include <mapbox/variant.hpp>

namespace mapbox {
namespace geojson {

// Use the CrtAllocator, because the MemoryPoolAllocator is broken on ARM
// https://github.com/miloyip/rapidjson/issues/200, 301, 388
using rapidjson_allocator = rapidjson::CrtAllocator;
using rapidjson_document  = rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson_allocator>;
using rapidjson_value     = rapidjson::GenericValue<rapidjson::UTF8<>, rapidjson_allocator>;
using error               = std::runtime_error;

template <typename T>
using geojson_base = mapbox::util::
    variant<geometry::geometry<T>, geometry::feature<T>, geometry::feature_collection<T>>;

template <typename T>
struct geojson : geojson_base<T> {
    using coordinate_type = T;
    using geojson_base<T>::geojson_base;

    geojson() = delete;
};

// Stringify inputs of known types. Instantiations are provided for geometry, feature,
// feature_collection, and geojson variant.
template <typename T>
std::string stringify(const T &);

// Parsing of strings to objects. Instantiations are provided for geometry, feature,
// feature_collection, and geojson variant.
template <typename T>
geometry::geometry<T> parse_geometry(const std::string &json);
template <typename T>
geometry::feature<T> parse_feature(const std::string &json);
template <typename T>
geometry::feature_collection<T> parse_feature_collection(const std::string &json);
template <typename T>
geojson<T> parse(const std::string &json);

template <typename T>
T convert(const rapidjson_value &json);

template <typename T>
geometry::point<T> convert_point(const rapidjson_value &json) {
    if (json.Size() < 2) {
        throw error("coordinates array must have at least 2 numbers");
    }
    return geometry::point<T>{ static_cast<T>(json[0].GetDouble()),
                               static_cast<T>(json[1].GetDouble()) };
}

template <>
geometry::point<std::int64_t> convert_point<std::int64_t>(const rapidjson_value &json) {
    if (json.Size() < 2) {
        throw error("coordinates array must have at least 2 numbers");
    }
    return geometry::point<std::int64_t>{ json[0].GetInt64(), json[1].GetInt64() };
}

template <>
geometry::point<std::uint64_t> convert_point<std::uint64_t>(const rapidjson_value &json) {
    if (json.Size() < 2) {
        throw error("coordinates array must have at least 2 numbers");
    }
    return geometry::point<std::uint64_t>{ json[0].GetUint64(), json[1].GetUint64() };
}

template <typename T>
geometry::multi_point<T> convert_multi_point(const rapidjson_value &json) {
    geometry::multi_point<T> points;
    auto size = json.Size();
    points.reserve(size);

    for (auto &element : json.GetArray()) {
        points.push_back(convert_point<T>(element));
    }
    return points;
}

template <typename T>
geometry::line_string<T> convert_line_string(const rapidjson_value &json) {
    geometry::line_string<T> points;
    auto size = json.Size();
    points.reserve(size);

    for (auto &element : json.GetArray()) {
        points.push_back(convert_point<T>(element));
    }
    return points;
}

template <typename T>
geometry::linear_ring<T> convert_linear_ring(const rapidjson_value &json) {
    geometry::linear_ring<T> points;
    auto size = json.Size();
    points.reserve(size);

    for (auto &element : json.GetArray()) {
        points.push_back(convert_point<T>(element));
    }
    return points;
}

template <typename T>
geometry::polygon<T> convert_polygon(const rapidjson_value &json) {
    geometry::polygon<T> points;
    auto size = json.Size();
    points.reserve(size);

    for (auto &element : json.GetArray()) {
        points.push_back(convert_linear_ring<T>(element));
    }
    return points;
}

template <typename T>
geometry::multi_polygon<T> convert_multi_polygon(const rapidjson_value &json) {
    geometry::multi_polygon<T> points;
    auto size = json.Size();
    points.reserve(size);

    for (auto &element : json.GetArray()) {
        points.push_back(convert_polygon<T>(element));
    }
    return points;
}

template <typename T>
geometry::multi_line_string<T> convert_multi_line_string(const rapidjson_value &json) {
    geometry::multi_line_string<T> points;
    auto size = json.Size();
    points.reserve(size);

    for (auto &element : json.GetArray()) {
        points.push_back(convert_line_string<T>(element));
    }
    return points;
}

template <typename T>
geometry::geometry<T> convert_geometry(const rapidjson_value &json);

template <typename T>
geometry::geometry_collection<T> convert_geometry_collection(const rapidjson_value &json) {
    geometry::geometry_collection<T> points;
    auto size = json.Size();
    points.reserve(size);

    for (auto &element : json.GetArray()) {
        points.push_back(convert_geometry<T>(element));
    }
    return points;
}

template <typename T>
geometry::geometry<T> convert_geometry(const rapidjson_value &json) {
    if (!json.IsObject()) {
        throw error("Geometry must be an object");
    }
    const auto &json_end = json.MemberEnd();

    const auto &type_itr = json.FindMember("type");
    if (type_itr == json_end) {
        throw error("Geometry must have a type property");
    }
    const auto &type = type_itr->value;

    if (type == "GeometryCollection") {
        const auto &geometries_itr = json.FindMember("geometries");
        if (geometries_itr == json_end) {
            throw error("GeometryCollection must have a geometries property");
        }

        const auto &json_geometries = geometries_itr->value;

        if (!json_geometries.IsArray()) {
            throw error("GeometryCollection geometries property must be an array");
        }
        return geometry::geometry<T>{ convert_geometry_collection<T>(json_geometries) };
    }

    const auto &coords_itr = json.FindMember("coordinates");

    if (coords_itr == json_end) {
        throw error(std::string(type.GetString()) + " geometry must have a coordinates property");
    }

    const auto &json_coords = coords_itr->value;
    if (!json_coords.IsArray()) {
        throw error("coordinates property must be an array");
    }
    if (type == "Point") {
        return geometry::geometry<T>{ convert_point<T>(json_coords) };
    }
    if (type == "MultiPoint") {
        return geometry::geometry<T>{ convert_multi_point<T>(json_coords) };
    }
    if (type == "LineString") {
        return geometry::geometry<T>{ convert_line_string<T>(json_coords) };
    }
    if (type == "MultiLineString") {
        return geometry::geometry<T>{ convert_multi_line_string<T>(json_coords) };
    }
    if (type == "Polygon") {
        return geometry::geometry<T>{ convert_polygon<T>(json_coords) };
    }
    if (type == "MultiPolygon") {
        return geometry::geometry<T>{ convert_multi_polygon<T>(json_coords) };
    }

    throw error(std::string(type.GetString()) + " not yet implemented");
}

template <>
geometry::value convert<geometry::value>(const rapidjson_value &json);

template <>
std::vector<geometry::value> convert<std::vector<geometry::value>>(const rapidjson_value &json) {
    std::vector<geometry::value> points;
    auto size = json.Size();
    points.reserve(size);

    for (auto &element : json.GetArray()) {
        points.push_back(convert<geometry::value>(element));
    }
    return points;
}

template <>
geometry::property_map convert<geometry::property_map>(const rapidjson_value &json) {
    if (!json.IsObject())
        throw error("properties must be an object");

    geometry::property_map result;
    for (auto &member : json.GetObject()) {
        result.emplace(std::string(member.name.GetString(), member.name.GetStringLength()),
                       convert<geometry::value>(member.value));
    }
    return result;
}

template <>
geometry::value convert<geometry::value>(const rapidjson_value &json) {
    switch (json.GetType()) {
    case rapidjson::kNullType:
        return nullptr;
    case rapidjson::kFalseType:
        return false;
    case rapidjson::kTrueType:
        return true;
    case rapidjson::kObjectType:
        return convert<geometry::property_map>(json);
    case rapidjson::kArrayType:
        return convert<std::vector<geometry::value>>(json);
    case rapidjson::kStringType:
        return std::string(json.GetString(), json.GetStringLength());
    default:
        assert(json.GetType() == rapidjson::kNumberType);
        if (json.IsUint64()) {
            return std::uint64_t(json.GetUint64());
        }
        if (json.IsInt64()) {
            return std::int64_t(json.GetInt64());
        }
        return json.GetDouble();
    }
}

template <>
geometry::identifier convert<geometry::identifier>(const rapidjson_value &json) {
    switch (json.GetType()) {
    case rapidjson::kStringType:
        return std::string(json.GetString(), json.GetStringLength());
    case rapidjson::kNumberType:
        if (json.IsUint64()) {
            return std::uint64_t(json.GetUint64());
        }
        if (json.IsInt64()) {
            return std::int64_t(json.GetInt64());
        }
        return json.GetDouble();
    default:
        throw error("Feature id must be a string or number");
    }
}

template <typename T>
geometry::feature<T> convert_feature(const rapidjson_value &json) {
    if (!json.IsObject()) {
        throw error("Feature must be an object");
    }

    auto const &json_end = json.MemberEnd();
    auto const &type_itr = json.FindMember("type");

    if (type_itr == json_end) {
        throw error("Feature must have a type property");
    }
    if (type_itr->value != "Feature") {
        throw error("Feature type must be Feature");
    }

    auto const &geom_itr = json.FindMember("geometry");

    if (geom_itr == json_end) {
        throw error("Feature must have a geometry property");
    }

    geometry::feature<T> result{ convert_geometry<T>(geom_itr->value) };

    auto const &id_itr = json.FindMember("id");
    if (id_itr != json_end) {
        result.id = convert<geometry::identifier>(id_itr->value);
    }

    auto const &prop_itr = json.FindMember("properties");

    if (prop_itr == json_end) {
        throw error("Feature must have a properties property");
    }

    const auto &json_props = prop_itr->value;
    if (!json_props.IsNull()) {
        result.properties = convert<geometry::property_map>(json_props);
    }

    return result;
}

template <typename T>
geometry::feature_collection<T> convert_feature_collection(const rapidjson_value &json) {
    if (!json.IsArray()) {
        throw error("FeatureCollection features property must be an array");
    }
    geometry::feature_collection<T> points;
    auto size = json.Size();
    points.reserve(size);

    for (auto &element : json.GetArray()) {
        points.push_back(convert_feature<T>(element));
    }
    return points;
}

template <typename T>
geojson<T> convert_geojson(const rapidjson_value &json) {
    if (!json.IsObject()) {
        throw error("GeoJSON must be an object");
    }

    const auto &type_itr = json.FindMember("type");
    const auto &json_end = json.MemberEnd();

    if (type_itr == json_end) {
        throw error("GeoJSON must have a type property");
    }

    const auto &type = type_itr->value;

    if (type == "FeatureCollection") {
        const auto &features_itr = json.FindMember("features");
        if (features_itr == json_end) {
            throw error("FeatureCollection must have features property");
        }

        const auto &json_features = features_itr->value;

        return geojson<T>{ convert_feature_collection<T>(json_features) };
    }

    if (type == "Feature") {
        return geojson<T>{ convert_feature<T>(json) };
    }

    return geojson<T>{ convert_geometry<T>(json) };
}

template <typename T>
geometry::geometry<T> parse_geometry(const std::string &json) {
    rapidjson_document d;
    d.Parse(json.c_str());
    return convert_geometry<T>(d);
}

template <typename T>
geometry::feature<T> parse_feature(const std::string &json) {
    rapidjson_document d;
    d.Parse(json.c_str());
    return convert_feature<T>(d);
}

template <typename T>
geometry::feature_collection<T> parse_feature_collection(const std::string &json) {
    rapidjson_document d;
    d.Parse(json.c_str());
    return convert_feature_collection<T>(d);
}

template <typename T>
geojson<T> parse(const std::string &json) {
    rapidjson_document d;
    d.Parse(json.c_str());
    return convert_geojson<T>(d);
}

template <typename T>
struct to_type {
public:
    const char *operator()(const geometry::point<T> &) {
        return "Point";
    }

    const char *operator()(const geometry::line_string<T> &) {
        return "LineString";
    }

    const char *operator()(const geometry::polygon<T> &) {
        return "Polygon";
    }

    const char *operator()(const geometry::multi_point<T> &) {
        return "MultiPoint";
    }

    const char *operator()(const geometry::multi_line_string<T> &) {
        return "MultiLineString";
    }

    const char *operator()(const geometry::multi_polygon<T> &) {
        return "MultiPolygon";
    }

    const char *operator()(const geometry::geometry_collection<T> &) {
        return "GeometryCollection";
    }
};

template <typename T>
rapidjson_value convert(const geometry::geometry<T> &element, rapidjson_allocator &allocator);

template <typename T>
struct to_coordinates_or_geometries {
    rapidjson_allocator &allocator;

    // Handles line_string, polygon, multi_point, multi_line_string, multi_polygon, and
    // geometry_collection.
    template <class E>
    rapidjson_value operator()(const std::vector<E> &vector) {
        rapidjson_value result;
        result.SetArray();
        for (std::size_t i = 0; i < vector.size(); ++i) {
            result.PushBack(operator()(vector[i]), allocator);
        }
        return result;
    }

    rapidjson_value operator()(const geometry::point<T> &element) {
        rapidjson_value result;
        result.SetArray();
        result.PushBack(element.x, allocator);
        result.PushBack(element.y, allocator);
        return result;
    }

    rapidjson_value operator()(const geometry::geometry<T> &element) {
        return convert<T>(element, allocator);
    }
};

struct to_value {
    rapidjson_allocator &allocator;

    rapidjson_value operator()(geometry::null_value_t) {
        rapidjson_value result;
        result.SetNull();
        return result;
    }

    rapidjson_value operator()(bool t) {
        rapidjson_value result;
        result.SetBool(t);
        return result;
    }

    rapidjson_value operator()(int64_t t) {
        rapidjson_value result;
        result.SetInt64(t);
        return result;
    }

    rapidjson_value operator()(uint64_t t) {
        rapidjson_value result;
        result.SetUint64(t);
        return result;
    }

    rapidjson_value operator()(double t) {
        rapidjson_value result;
        result.SetDouble(t);
        return result;
    }

    rapidjson_value operator()(const std::string &t) {
        rapidjson_value result;
        result.SetString(t.data(), rapidjson::SizeType(t.size()), allocator);
        return result;
    }

    rapidjson_value operator()(const std::vector<geometry::value> &array) {
        rapidjson_value result;
        result.SetArray();
        for (const auto &item : array) {
            result.PushBack(geometry::value::visit(item, *this), allocator);
        }
        return result;
    }

    rapidjson_value operator()(const std::unordered_map<std::string, geometry::value> &map) {
        rapidjson_value result;
        result.SetObject();
        for (const auto &property : map) {
            result.AddMember(
                rapidjson::GenericStringRef<char>{ property.first.data(),
                                                   rapidjson::SizeType(property.first.size()) },
                geometry::value::visit(property.second, *this), allocator);
        }
        return result;
    }
};

template <typename T>
rapidjson_value convert(const geometry::geometry<T> &element, rapidjson_allocator &allocator) {
    rapidjson_value result(rapidjson::kObjectType);

    result.AddMember("type", rapidjson::GenericStringRef<char>{ geometry::geometry<T>::visit(
                                 element, to_type<T>()) },
                     allocator);

    result.AddMember(
        rapidjson::GenericStringRef<char>{ element.template is<geometry::geometry_collection<T>>()
                                               ? "geometries"
                                               : "coordinates" },
        geometry::geometry<T>::visit(element, to_coordinates_or_geometries<T>{ allocator }),
        allocator);

    return result;
}

template <typename T>
rapidjson_value convert(const geometry::feature<T> &element, rapidjson_allocator &allocator) {
    rapidjson_value result(rapidjson::kObjectType);
    result.AddMember("type", "Feature", allocator);

    if (element.id) {
        result.AddMember("id", geometry::identifier::visit(*element.id, to_value{ allocator }),
                         allocator);
    }

    result.AddMember("geometry", convert<T>(element.geometry, allocator), allocator);
    result.AddMember("properties", to_value{ allocator }(element.properties), allocator);

    return result;
}

template <typename T>
rapidjson_value convert(const geometry::feature_collection<T> &collection,
                        rapidjson_allocator &allocator) {
    rapidjson_value result(rapidjson::kObjectType);
    result.AddMember("type", "FeatureCollection", allocator);

    rapidjson_value features(rapidjson::kArrayType);
    for (const auto &element : collection) {
        features.PushBack(convert<T>(element, allocator), allocator);
    }
    result.AddMember("features", features, allocator);

    return result;
}

template <typename T>
rapidjson_value convert(const geojson<T> &element, rapidjson_allocator &allocator) {
    return geojson<T>::visit(
        element, [&](const auto &alternative) { return convert<T>(alternative, allocator); });
}

template <typename T>
std::string stringify(const geometry::geometry<T> &t) {
    rapidjson_allocator allocator;
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    convert<T>(t, allocator).Accept(writer);
    return buffer.GetString();
}

template <typename T>
std::string stringify(const geometry::feature<T> &t) {
    rapidjson_allocator allocator;
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    convert<T>(t, allocator).Accept(writer);
    return buffer.GetString();
}

template <typename T>
std::string stringify(const geometry::feature_collection<T> &t) {
    rapidjson_allocator allocator;
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    convert<T>(t, allocator).Accept(writer);
    return buffer.GetString();
}

template <typename T>
std::string stringify(const geojson<T> &element) {
    return geojson<T>::visit(element,
                             [](const auto &alternative) { return stringify<T>(alternative); });
}

} // namespace geojson
} // namespace mapbox
