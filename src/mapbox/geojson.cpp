#include <mapbox/geojson.hpp>
#include <mapbox/geojson/rapidjson.hpp>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace mapbox {
namespace geojson {

using error    = std::runtime_error;
using prop_map = std::unordered_map<std::string, value>;

template <typename T>
T convert(const rapidjson_value &json);

template <>
point convert<point>(const rapidjson_value &json) {
    if (json.Size() < 2)
        throw error("coordinates array must have at least 2 numbers");

    return point{ json[0].GetDouble(), json[1].GetDouble() };
}

template <typename Cont>
Cont convert(const rapidjson_value &json) {
    Cont points;
    auto size = json.Size();
    points.reserve(size);

    for (rapidjson::SizeType i = 0; i < size; ++i) {
        points.push_back(convert<typename Cont::value_type>(json[i]));
    }
    return points;
}

template <>
geometry convert<geometry>(const rapidjson_value &json) {
    if (!json.IsObject())
        throw error("Geometry must be an object");

    const auto &json_end = json.MemberEnd();

    const auto &type_itr = json.FindMember("type");
    if (type_itr == json_end)
        throw error("Geometry must have a type property");

    const auto &type = type_itr->value;

    if (type == "GeometryCollection") {
        const auto &geometries_itr = json.FindMember("geometries");
        if (geometries_itr == json_end)
            throw error("GeometryCollection must have a geometries property");

        const auto &json_geometries = geometries_itr->value;

        if (!json_geometries.IsArray())
            throw error("GeometryCollection geometries property must be an array");

        return geometry{ convert<geometry_collection>(json_geometries) };
    }

    const auto &coords_itr = json.FindMember("coordinates");

    if (coords_itr == json_end)
        throw error(std::string(type.GetString()) + " geometry must have a coordinates property");

    const auto &json_coords = coords_itr->value;
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
value convert<value>(const rapidjson_value &json);

template <>
prop_map convert(const rapidjson_value &json) {
    if (!json.IsObject())
        throw error("properties must be an object");

    prop_map result;
    for (auto itr = json.MemberBegin(); itr != json.MemberEnd(); ++itr) {
        result.emplace(std::string(itr->name.GetString(), itr->name.GetStringLength()),
                       convert<value>(itr->value));
    }
    return result;
}

template <>
value convert<value>(const rapidjson_value &json) {
    switch (json.GetType()) {
    case rapidjson::kNullType:
        return nullptr;
    case rapidjson::kFalseType:
        return false;
    case rapidjson::kTrueType:
        return true;
    case rapidjson::kObjectType:
        return convert<prop_map>(json);
    case rapidjson::kArrayType:
        return convert<std::vector<value>>(json);
    case rapidjson::kStringType:
        return std::string(json.GetString(), json.GetStringLength());
    default:
        assert(json.GetType() == rapidjson::kNumberType);
        if (json.IsUint64())
            return std::uint64_t(json.GetUint64());
        if (json.IsInt64())
            return std::int64_t(json.GetInt64());
        return json.GetDouble();
    }
}

template <>
feature convert<feature>(const rapidjson_value &json) {
    if (!json.IsObject())
        throw error("Feature must be an object");

    auto const &json_end = json.MemberEnd();
    auto const &type_itr = json.FindMember("type");

    if (type_itr == json_end)
        throw error("Feature must have a type property");
    if (type_itr->value != "Feature")
        throw error("Feature type must be Feature");

    auto const &geom_itr = json.FindMember("geometry");

    if (geom_itr == json_end)
        throw error("Feature must have a geometry property");

    feature feature{ convert<geometry>(geom_itr->value) };

    auto const &prop_itr = json.FindMember("properties");

    if (prop_itr == json_end)
        throw error("Feature must have a properties property");

    const auto &json_props = prop_itr->value;
    if (!json_props.IsNull()) {
        feature.properties = convert<prop_map>(json_props);
    }

    return feature;
}

template <>
geojson convert<geojson>(const rapidjson_value &json) {
    if (!json.IsObject())
        throw error("GeoJSON must be an object");

    const auto &type_itr = json.FindMember("type");
    const auto &json_end = json.MemberEnd();

    if (type_itr == json_end)
        throw error("GeoJSON must have a type property");

    const auto &type = type_itr->value;

    if (type == "FeatureCollection") {
        const auto &features_itr = json.FindMember("features");
        if (features_itr == json_end)
            throw error("FeatureCollection must have features property");

        const auto &json_features = features_itr->value;

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
    rapidjson_document d;
    d.Parse(json.c_str());
    return convert<T>(d);
}

template <>
geometry parse<geometry>(const std::string &);
template <>
feature parse<feature>(const std::string &);
template <>
feature_collection parse<feature_collection>(const std::string &);

geojson parse(const std::string &json) {
    return parse<geojson>(json);
}

geojson convert(const rapidjson_value &json) {
    return convert<geojson>(json);
}

using Writer = rapidjson::Writer<rapidjson::StringBuffer>;

void write(const geometry&, Writer&);
void write(const feature&, Writer&);
void write(const feature_collection&, Writer&);

struct to_type {
public:
    const char * operator()(const point&) {
        return "Point";
    }

    const char * operator()(const line_string&) {
        return "LineString";
    }

    const char * operator()(const polygon&) {
        return "Polygon";
    }

    const char * operator()(const multi_point&) {
        return "MultiPoint";
    }

    const char * operator()(const multi_line_string&) {
        return "MultiLineString";
    }

    const char * operator()(const multi_polygon&) {
        return "MultiPolygon";
    }

    const char * operator()(const geometry_collection&) {
        return "GeometryCollection";
    }
};

struct write_coordinates_or_geometries {
    Writer& writer;

    // Handles line_string, polygon, multi_point, multi_line_string, multi_polygon, and geometry_collection.
    template <class E>
    void operator()(const std::vector<E>& vector) {
        writer.StartArray();
        for (std::size_t i = 0; i < vector.size(); ++i) {
            operator()(vector[i]);
        }
        writer.EndArray();
    }

    void operator()(const point& point) {
        writer.StartArray();
        writer.Double(point.x);
        writer.Double(point.y);
        writer.EndArray();
    }

    void operator()(const geometry& geometry) {
        return write(geometry, writer);
    }
};

struct write_value {
    Writer& writer;

    void operator()(null_value_t) {
        writer.Null();
    }

    void operator()(bool t) {
        writer.Bool(t);
    }

    void operator()(int64_t t) {
        writer.Int64(t);
    }

    void operator()(uint64_t t) {
        writer.Uint64(t);
    }

    void operator()(double t) {
        writer.Double(t);
    }

    void operator()(const std::string& t) {
        writer.String(t.data(), t.size());
    }

    void operator()(const std::vector<value>& array) {
        writer.StartArray();
        for (const auto& value : array) {
            value::visit(value, *this);
        }
        writer.EndArray();
    }

    void operator()(const std::unordered_map<std::string, value>& map) {
        writer.StartObject();
        for (const auto& property : map) {
            writer.Key(property.first.data(), property.first.size());
            value::visit(property.second, write_value { writer });
        }
        writer.EndObject();
    }
};

void write(const geometry& geometry, Writer& writer) {
    writer.StartObject();

    writer.Key("type");
    writer.String(geometry::visit(geometry, to_type()));

    writer.Key(geometry.is<geometry_collection>() ? "geometries" : "coordinates");
    geometry::visit(geometry, write_coordinates_or_geometries { writer });

    writer.EndObject();
}

void write(const feature& feature, Writer& writer) {
    writer.StartObject();

    writer.Key("type");
    writer.String("Feature");

    if (feature.id) {
        writer.Key("id");
        identifier::visit(*feature.id, write_value { writer });
    }

    writer.Key("geometry");
    write(feature.geometry, writer);

    writer.Key("properties");
    write_value { writer }(feature.properties);

    writer.EndObject();
}

void write(const feature_collection& feature_collection, Writer& writer) {
    writer.StartObject();

    writer.Key("type");
    writer.String("FeatureCollection");

    writer.Key("features");
    writer.StartArray();
    for (const auto& feature : feature_collection) {
        write(feature, writer);
    }
    writer.EndArray();

    writer.EndObject();
}

template <class T>
std::string stringify(const T& t) {
    rapidjson::StringBuffer s;
    Writer writer(s);
    write(t, writer);
    return s.GetString();
}

std::string stringify(const geojson& geojson) {
    return geojson::visit(geojson, [] (const auto& alternative) {
        return stringify(alternative);
    });
}

} // namespace geojson
} // namespace mapbox
