#ifndef INCLUDED_STATS
#define INCLUDED_STATS

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

struct Stats {
	double time_per_point = 0;
	double time_total = 0;
	double log_score = 0;

	void dump_into_dom(rapidjson::Value &val, rapidjson::MemoryPoolAllocator<> &alloc);
};

#endif //ndef INCLUDED_STATS