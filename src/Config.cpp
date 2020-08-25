#define _CRT_SECURE_NO_WARNINGS // Visual Studio gives deprecation ERROR on fopen :(

#include "Config.h"

#include <iostream>
#include <string>
using std::string;

#include <string_view>
using std::string_view;

#include <sstream>
using std::istringstream;
using std::ostringstream;

#include "Logging.h"
#include "write_geojson.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"
#include <cstdio>

// Somehow, a macro leaks into our code in Visual Studio :(
#undef GetObject

void Config::try_force_png() {
	if (force_tiff) {
		console->warn("Forcing PNG, overriding previously request to force TIFF.");
		force_tiff = false;
	}
	force_png = true;
}
void Config::try_force_tiff() {
	if (force_png) {
		console->warn("Forcing TIFF, overriding previously request to force PNG.");
		force_png = false;
	}
	force_tiff = true;
}

void handle_docopt_integer(string_view message, int &result, const docopt::value &val) {
	if (val) {
		if (val.isLong()) {
			// does not seem to happen; it's a string.
			result = val.asLong();
		}
		else if (val.isString()) {
			string s = val.asString();
			size_t pos = s.length();
			int i = std::stoi(s, &pos);
			if (pos == s.length()) {
				result = i;
			}
			else {
				console->warn("Expected integer for '{}' but could not interpret \"{}\"; ignored.", message, s);
			}
		}
		else if (val.isBool()) {
			console->warn("Expected integer for '{}' but got boolean {}; ignored.", message, val.asBool());
		}
	}
	// Do not do anything if value is missing; this is not a warning.
}
void handle_docopt_double(string_view message, double &result, const docopt::value &val) {
	if (val) {
		if (val.isLong()) {
			// does not seem to happen; it's a string.
			result = val.asLong();
		}
		else if (val.isString()) {
			string s = val.asString();
			size_t pos = s.length();
			double d = std::stod(s, &pos);
			if (pos == s.length()) {
				result = d;
			}
			else {
				console->warn("Expected number for '{}' but could not interpret \"{}\"; ignored.", message, s);
			}
		}
		else if (val.isBool()) {
			console->warn("Expected number for '{}' but got boolean {}; ignored.", message, val.asBool());
		}
	}
	// Do not do anything if value is missing; this is not a warning.
}
void Config::ingest_docopt(std::map<std::string, docopt::value> &args) {
	docopt::value &val = args["<image>"];
	if (val.isString()) image_filename = val.asString();

	val = args["<linestring>"];
	if (val.isString()) linestring_filename = val.asString();

	val = args["--output"];
	if (val.isString()) output_filename = val.asString();

	val = args["--pointer"];
	if (val.isString()) geojson_pointer = val.asString();

	val = args["--tag"];
	if (val.isString()) tag_string = val.asString();

	val = args["--debug-image"];
	if (val.isString()) debug_image_filename = val.asString();

	handle_docopt_integer("--max-displace", window_size, args["--max-displace"]);
	handle_docopt_double("--kernel-sigma", global_sigma, args["--kernel-sigma"]);
	handle_docopt_integer("--stride", linestring_stride, args["--stride"]);
	handle_docopt_integer("--subdivide", linestring_subdivide, args["--subdivide"]);

	if (args["--output-full-dom"].asBool()) output_full_dom = true;

	if (args["--png"].asBool()) try_force_png();
	if (args["--tiff"].asBool()) try_force_tiff();
}

void handle_json_integer(string_view filename, string_view message, int &result, rapidjson::Value &val) {
	if (val.IsInt()) {
		result = val.GetInt();
	}
	else {
		console->warn("{}: Expected integer for '{}' but could not interpret; ignored.", filename, message);
	}
}
void handle_json_double(string_view filename, string_view message, double &result, rapidjson::Value &val) {
	if (val.IsDouble()) {
		result = val.GetDouble();
	}
	else if (val.IsInt()) { // can this happen?
		result = val.GetInt();
	}
	else {
		console->warn("{}: Expected number for '{}' but could not interpret; ignored.", filename, message);
	}
}
void handle_json_string(string_view filename, string_view message, string &result, rapidjson::Value &val) {
	if (val.IsString()) {
		result = val.GetString();
	}
	else {
		console->warn("{}: Expected string for '{}' but got something else; ignored.", filename, message);
	}
}
void Config::ingest_json(const std::string &filename) {
	console->info("Reading config from \"{}\"  ...", filename);
	FILE *fp = fopen(filename.c_str(), "rb");
	if (fp == 0) {
		console->warn("Failed to open \"{}\" for config; ignored.", filename);
		return;
	}
	char buffer[65536];
	rapidjson::FileReadStream stream(fp, buffer, sizeof(buffer));
	rapidjson::Document dom;
	dom.ParseStream(stream);
	fclose(fp);
	if (dom.HasParseError()) {
		console->warn("Config file JSON parse error ({}:{}): {}", filename, dom.GetErrorOffset(), rapidjson::GetParseError_En(dom.GetParseError()));
		return;
	}
	for (auto &m : dom.GetObject()) {
		string_view key = m.name.GetString();
		if (key == "image") {
			handle_json_string(filename, "image", image_filename, m.value);
		}
		else if (key == "linestring") {
			handle_json_string(filename, "linestring", linestring_filename, m.value);
		}
		else if (key == "output") {
			handle_json_string(filename, "output", linestring_filename, m.value);
		}
		else if (key == "debug_image") {
			handle_json_string(filename, "debug_image", debug_image_filename, m.value);
		}
		else if (key == "pointer") {
			handle_json_string(filename, "pointer", geojson_pointer, m.value);
		}
		else if (key == "tag") {
			handle_json_string(filename, "tag", tag_string, m.value);
		}
		else if (key == "max_displace") {
			handle_json_integer(filename, "max_displace", window_size, m.value);
		}
		else if (key == "kernel_sigma") {
			handle_json_double(filename, "kernel_sigma", global_sigma, m.value);
		}
		else if (key == "stride") {
			handle_json_integer(filename, "stride", linestring_stride, m.value);
		}
		else if (key == "subdivide") {
			handle_json_integer(filename, "subsample", linestring_subdivide, m.value);
		}
		else if (key == "output_full_dom") {
			output_full_dom = m.value.IsTrue();
		}
		else if (key == "png") {
			if (m.value.IsTrue()) {
				try_force_png();
			}
			else {
				console->warn("The JSON attribute \"png\" may only have value true. Got something else; ignored.");
			}
		}
		else if (key == "tiff") {
			if (m.value.IsTrue()) {
				try_force_png();
			}
			else {
				console->warn("The JSON attribute \"tiff\" may only have value true. Got something else; ignored.");
			}
		}
		else {
			console->warn("Did not recognise JSON attribute \"{}\"; ignored.", key);
		}
	}
}

void Config::dump_to_console() {
	{
		string forced_mode = "";
		if (force_tiff) forced_mode = " (Forced TIFF mode)";
		if (force_png) forced_mode = " (Forced PNG mode.)";
		console->info("Image            : {}{}", image_filename, forced_mode);
	}
	console->info("Output file      : {}", output_filename);
	console->info("Polyline         : {}", linestring_filename);
	console->info("Max displacement : {}", window_size);
	console->info("Kernel sigma     : {}", global_sigma);
	console->info("Polyline stride  : {}", linestring_stride);
}

void Config::dump_into_dom(rapidjson::Value &obj, rapidjson::MemoryPoolAllocator<> &alloc) {
	using namespace rapidjson;
	obj.SetObject();
	if (tag_string != "") obj.AddMember("tag", Value(tag_string.c_str(), alloc).Move(), alloc);
	if (debug_image_filename != "") obj.AddMember("debug_image", Value(debug_image_filename.c_str(), alloc).Move(), alloc);
	obj.AddMember("image", Value(image_filename.c_str(), alloc).Move(), alloc);
	obj.AddMember("linestring", Value(linestring_filename.c_str(), alloc).Move(), alloc);
	obj.AddMember("output", Value(output_filename.c_str(), alloc).Move(), alloc);
	obj.AddMember("pointer", Value(geojson_pointer.c_str(), alloc).Move(), alloc);
	obj.AddMember("max_displace", window_size, alloc);
	obj.AddMember("kernel_sigma", global_sigma, alloc);
	obj.AddMember("stride", linestring_stride, alloc);
	obj.AddMember("subdivide", linestring_subdivide, alloc);
	if (output_full_dom) obj.AddMember("output_full_dom", true, alloc);
	if (force_png) obj.AddMember("png", true, alloc);
	if (force_tiff) obj.AddMember("tiff", true, alloc);
}

void Config::json_to_stdout() {
	// build DOM
	console->info("Constructing configuration DOM for output ...");
	rapidjson::Document dom;
	dom.SetObject();
	dump_into_dom(dom, dom.GetAllocator());

	write_json(dom, std::cout);
}

string Config::settings_tag() {
	ostringstream tag;
	tag << "-out-w" << window_size << "-sigma" << global_sigma << "-stride" << linestring_stride;
	return tag.str();
}
