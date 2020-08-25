#include "write_geojson.h"

#include <iostream>
using std::ostream;

#include <functional>
using std::function;

#include "rapidjson/pointer.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"
using namespace rapidjson;

void write_json( Value &value, ostream &stream) {
	rapidjson::OStreamWrapper stdout_wrapper(stream);
	rapidjson::PrettyWriter writer(stdout_wrapper);
	writer.SetFormatOptions(PrettyFormatOptions::kFormatSingleLineArray);
	writer.SetIndent(' ', 2);
	value.Accept(writer);
	stream << '\n';
}

void replace_coords_array(Document::AllocatorType &alloc, rapidjson::Value &coords, const LineString &line_string, function<void(double*,double*)> transform) {
	coords.Clear();
	for (auto p : line_string.opt) {
		double x = p.x + 0.5;
		double y = p.y + 0.5;
		if(transform) transform(&x, &y);
		else y = -y;
		Value coord;
		coord.SetArray();
		coord.PushBack(x, alloc);
		coord.PushBack(y, alloc);
		coords.PushBack(coord, alloc);
	}
}

void write_geojson(rapidjson::Document &dom, Config &config, Stats &stats, const LineString &line_string, ostream &stream, function<void(double*,double*)> transform) {
	Value *root = Pointer(config.geojson_pointer.data()).Get(dom);
	Value &feature_type = Pointer("/geometry/type").Create(*root,dom.GetAllocator());
	feature_type.SetString("LineString");
	Value &coords = *Pointer("/geometry/coordinates").Get(*root);
	replace_coords_array(dom.GetAllocator(), coords, line_string, transform);
	Value &config_object = Pointer("/properties/align/config").Create(*root, dom.GetAllocator());
	config_object.SetObject();
	config.dump_into_dom(config_object, dom.GetAllocator());
	Value &stats_object = Pointer("/properties/align/stats").Create(*root, dom.GetAllocator());
	stats.dump_into_dom(stats_object, dom.GetAllocator());
	if (config.output_full_dom) {
		write_json(dom, stream);
	}
	else {
		write_json(*root, stream);
	}
}