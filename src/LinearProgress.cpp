#include "LinearProgress.h"

#include "Logging.h"

using namespace std;

LinearProgress::LinearProgress(string_view task_name, string_view unit, int n) : task_name(task_name), unit(unit), ticks(0), n(n) {}

void LinearProgress::start() {
	start_time = chrono::system_clock::now();
}
void LinearProgress::tick() {
	++ticks;
	auto now = chrono::system_clock::now();
	auto wait = now - last_message_time;
	if (wait > chrono::seconds(1)) {
		last_message_time = now;
		message();
	}
}
void LinearProgress::done() {
	message();
}

void LinearProgress::message() {
	int progress = static_cast<int>(100.0 * static_cast<double>(ticks) / static_cast<double>(n));
	double per_second = static_cast<double>(ticks) / chrono::duration<double>(last_message_time - start_time).count();
	int remaining = static_cast<int>((n - ticks) / per_second);
	console->info("{0}{2:>3}%, {3:.2} {1}/sec, remaining: {4} s", task_name, unit, progress, per_second, remaining);
}