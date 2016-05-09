#include <mapbox/geometry.hpp>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <mapbox/geojson.hpp>

#include <cassert>
#include <cstdio>
#include <iostream>
#include <vector>

int main() {
    std::FILE *fp = std::fopen("test/fixtures/places.json", "r");
    char buffer[65536];
    rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));

    rapidjson::Document d;
    d.ParseStream(is);

    const auto &json_features = d["features"];

    mapbox::geometry::feature_collection<double> features;
    features.reserve(json_features.Size());

    for (auto itr = json_features.Begin(); itr != json_features.End(); ++itr) {
        const auto &json_coords = (*itr)["geometry"]["coordinates"];
        const auto lng = json_coords[0].GetDouble();
        const auto lat = json_coords[1].GetDouble();
        mapbox::geometry::point<double> point(lng, lat);
        mapbox::geometry::feature<double> feature{ point };
        features.push_back(feature);
    }
}
