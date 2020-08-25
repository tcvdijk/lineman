#define _CRT_SECURE_NO_WARNINGS

#include "load_geojson.h"

#include <string>
using std::string;

#include <string_view>
using std::string_view;

#include <functional>
using std::function;

using std::vector;
using std::filesystem::path;

#include <cstdio>

#include "Logging.h"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"
#include "rapidjson/pointer.h"

#include "proj_api.h"

vector<Point<Space::World>> load_geojson(rapidjson::Document &dom, const path &path, string_view geojson_pointer, int stride, int subdivide, function<void(double*, double*)> transform) {
	using namespace rapidjson;
	vector<Point<Space::World>> result;

	FILE* fp = fopen(path.string().c_str(), "rb");
	if (fp == 0) {
		console->error("Failed to open \"{}\" for geojson; aborting.", path.string());
		return result;
	}
	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	dom.ParseStream(is);
	fclose(fp);

	// Parse errors?
	if (dom.HasParseError()) {
		console->error("{}({}): {}", path.string(), dom.GetErrorOffset(), rapidjson::GetParseError_En(dom.GetParseError()));
		return result;
	}

	Pointer pointer(geojson_pointer.data());
	if (!pointer.IsValid()) {
		console->error("Invalid JSON pointer \"{}\"; error at character offset {}", geojson_pointer, pointer.GetParseErrorOffset());
		return result;
	}
	Value *root = pointer.Get(dom);
	if (root == nullptr) {
		console->error("Could not resolve JSON pointer \"{}\"; aborting.", geojson_pointer);
		return result;
	}
	string pointer_description = geojson_pointer == "" ? "JSON root" : fmt::format("GeoJSON element \"{}\"", geojson_pointer);
	if (!root->IsObject()) {
		console->error("{} is not an object.", pointer_description);
		if (geojson_pointer == "") {
			console->info("You can set a JSON pointer expression using --pointer. It should point to a LineString feature.");
		}
		return result;
	}
	Value *id = Pointer("/id").Get(*root);
	if (id) {
		if (id->IsString()) {
			console->info("{} has id attribute \"{}\".", pointer_description, id->GetString());
		}
		else if (id->IsInt()) {
			console->info("{} has id attribute {}.", pointer_description, id->GetInt());
		}
	}
	Value *name = Pointer("/properties/name").Get(*root);
	if (name && name->IsString()) {
		console->info("{} has name property \"{}\".", pointer_description, name->GetString());
	}

	if (!root->HasMember("type")) {
		console->warn("{} has no \"type\" attribute, expected \"Feature\".", pointer_description);
		if (geojson_pointer == "") {
			console->info("You can set a JSON pointer expression using --pointer. It should point to a LineString feature.");
		}
	}
	else if (string_view((*root)["type"].GetString()) != "Feature") {
		console->warn("{} has type \"{}\", expected \"Feature\".", pointer_description, (*root)["type"].GetString());
		if (geojson_pointer == "") {
			console->info("You can set a JSON pointer expression using --pointer. It should point to a LineString feature.");
		}
	}

	if (!root->HasMember("geometry")) {
		console->error("{} does not have a \"geometry\" attribute; aborting.", pointer_description);
		return result;
	}

	const Value &geometry = (*root)["geometry"];
	bool is_multi_linestring = false;
	if (!geometry.HasMember("type")) {
		console->warn("Geometry has no \"type\" attribute, expected \"LineString\".");
	}
	else if (string_view(geometry["type"].GetString()) != "LineString") {
		if (string_view(geometry["type"].GetString()) == "MultiLineString") {
			console->warn("Geometry type \"{}\", expected \"LineString\". Only processing first linestring!", geometry["type"].GetString());
			is_multi_linestring = true;
		}
		else {
			console->warn("Geometry type \"{}\", expected \"LineString\".", geometry["type"].GetString());
		}
	}

	if (!geometry.HasMember("coordinates")) {
		console->error("Geometry has no \"coordinates\" attribute; aborting.");
		return result;
	}

	if (!geometry["coordinates"].IsArray()) {
		console->error("Coordinates attribute is not an array; aborting.");
		return result;
	}
	if (is_multi_linestring && !geometry["coordinates"][0].IsArray()) {
		console->error("Expected MultiLineString but coordinates array is not an array of arrays.");
		return result;
	}
	const Value &coordinates = is_multi_linestring ? geometry["coordinates"][0] : geometry["coordinates"];


	int coord_index = -1;
	double prev_x = -1, prev_y = -1;
	for (const Value &p : coordinates.GetArray()) {
		++coord_index;
		if (stride > 0 && coord_index%stride != 0) {
			continue;
		}
		if (!p.IsArray()) {
			console->warn("Coordinates array element {} is not an array; ignoring.", coord_index);
			continue;
		}
		if (p.Size() != 2) {
			console->warn("Coordinates array element {} does not have length 2; ignoring.", coord_index);
			continue;
		}
		double x = p.GetArray()[0].GetDouble();
		double y = p.GetArray()[1].GetDouble();
		if (transform) transform(&x, &y);
		else y = -y;
		if (subdivide > 0 && !result.empty()) {
			double dx = (x - prev_x) / (subdivide + 1);
			double dy = (y - prev_y) / (subdivide + 1);
			for (int i = 1; i <= subdivide; ++i) {
				double subdiv_x = prev_x + i * dx;
				double subdiv_y = prev_y + i * dy;
				result.emplace_back(static_cast<int>(subdiv_x), static_cast<int>(subdiv_y));
			}
		}
		result.emplace_back(static_cast<int>(x), static_cast<int>(y));
		prev_x = x;
		prev_y = y;
	}

	return result;
}