#ifndef INCLUDED_LINEAR_PROGRESS
#define INCLUDED_LINEAR_PROGRESS

#include <string>
#include <string_view>
#include <chrono>

class LinearProgress {
public:
	LinearProgress(std::string_view task_name, std::string_view unit, int n);

	std::string task_name, unit;
	int ticks, n;

	void start();
	void tick();
	void done();

	void message();

	std::chrono::time_point<std::chrono::system_clock> start_time;
	std::chrono::time_point<std::chrono::system_clock> last_message_time;

};

#endif //ndef INCLUDED_LINEAR_PROGRESS
