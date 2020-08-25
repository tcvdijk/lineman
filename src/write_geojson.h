#ifndef INCLUDED_WRITE_GEOJSON
#define INCLUDED_WRITE_GEOJSON

#include <ostream>
#include <string_view>
#include <vector>

#include "rapidjson/document.h"

#include "Config.h"
#include "Stats.h"
#include "LineString.h"

void write_json(rapidjson::Value &value, std::ostream &stream);

void write_geojson(rapidjson::Document &dom, Config &config, Stats &stats, const LineString &line_string, std::ostream &stream, std::function<void(double*, double*)> transform);

#endif //ndef INCLUDED_GEOJSON