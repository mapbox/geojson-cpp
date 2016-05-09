#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

#include <mapbox/geojson.hpp>
#include <mapbox/geometry.hpp>

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

    const auto &data = mapbox::geojson::convert<mapbox::geojson::geojson>(d);
    assert(data.is<mapbox::geometry::feature_collection<double>>());
    return 0;
}
