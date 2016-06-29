#pragma once

#include <rapidjson/document.h>
#include <mapbox/geojson.hpp>

namespace mapbox {
namespace geojson {

// Use the CrtAllocator, because the MemoryPoolAllocator is broken on ARM
// https://github.com/miloyip/rapidjson/issues/200, 301, 388
using rapidjson_document = rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator>;
using rapidjson_value = rapidjson::GenericValue<rapidjson::UTF8<>, rapidjson::CrtAllocator>;

// Convert inputs of known types. Instantiations are provided for geometry, feature, and
// feature_collection.
template <typename T>
T convert(const rapidjson_value &json);

// Convert any GeoJSON type.
geojson convert(const rapidjson_value &json);

} // namespace geojson
} // namespace mapbox
