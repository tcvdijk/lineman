#ifndef INCLUDED_TIMER
#define INCLUDED_TIMER

#include <chrono>
#include <iostream>

class Timer {
public:

	Timer() {
		start_time = std::chrono::system_clock::now();
	}
	std::chrono::duration<double> elapsed() const {
		return (std::chrono::system_clock::now() - start_time);
	}

private:
	std::chrono::time_point<std::chrono::system_clock> start_time;

};

#endif //ndef INCLUDED_TIMER
