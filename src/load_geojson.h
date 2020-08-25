#ifndef INCLUDED_LOAD_GEOJSON
#define INCLUDED_LOAD_GEOJSON

#include <vector>
#include <filesystem>
#include <string_view>
#include <functional>

#include "Point.h"

std::vector<Point<Space::World>> load_geojson(rapidjson::Document &dom, const std::filesystem::path &path, std::string_view pointer, int stride, int subdivide, std::function<void(double*,double*)> transform);


#endif //ndef INCLUDED_LOAD_GEOJSON
