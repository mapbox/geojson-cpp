#pragma once

#include <mapbox/geojson.hpp>

namespace mapbox {
namespace geojson {

// Convert Value to known types. Instantiations are provided for geojson, geometry, feature, and
// feature_collection.
template <class T>
T convert(const mapbox::geojson::value &);

// Converts Value to GeoJSON type.
geojson convert(const mapbox::geojson::value&);

// Convert inputs of known types to Value. Instantiations are provided for geojson, geometry, feature, and
// feature_collection.
template <class T>
mapbox::geojson::value convert(const T &);

// Converts GeoJSON type to Value.
mapbox::geojson::value convert(const geojson&);

} // namespace geojson
} // namespace mapbox
