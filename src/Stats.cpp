#include "Stats.h"

void Stats::dump_into_dom(rapidjson::Value &obj, rapidjson::MemoryPoolAllocator<> &alloc) {
	using namespace rapidjson;
	obj.SetObject();
	obj.AddMember("runtime_total", time_total, alloc);
	obj.AddMember("time_per_point", time_per_point, alloc);
	obj.AddMember("log_score", log_score, alloc);
}