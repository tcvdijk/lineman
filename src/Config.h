#ifndef INCLUDED_CONFIG
#define INCLUDED_CONFIG

#include <string>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "docopt.h"

class Config {
public:
	
	std::string image_filename;
	std::string linestring_filename;
	std::string output_filename;
	std::string debug_image_filename;

	std::string geojson_pointer;

	std::string tag_string;

	bool force_png = false;
	bool force_tiff = false;
	void try_force_png();
	void try_force_tiff();

	int window_size = 15;
	double global_sigma = 10;
	int linestring_stride = 1;
	int linestring_subdivide = 0;

	bool output_full_dom = false;

	void ingest_docopt(std::map<std::string, docopt::value> &args);
	void ingest_json(const std::string &filename);

	void dump_into_dom(rapidjson::Value &val, rapidjson::MemoryPoolAllocator<> &alloc);
	void json_to_stdout();

	[[deprecated("Not up to date with all current config.")]]
	std::string settings_tag();

	[[deprecated("Not up to date with all current config.")]]
	void dump_to_console();

};

#endif //ndef INCLUDED_CONFIG