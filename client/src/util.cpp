#include "util.hpp"

#include <chrono>

float getDayProgress() {
	auto now = std::chrono::system_clock::now();
	time_t tnow = std::chrono::system_clock::to_time_t(now);
    tm *date = std::localtime(&tnow);
    date->tm_hour = 0;
    date->tm_min = 0;
    date->tm_sec = 0;
    auto midnight = std::chrono::system_clock::from_time_t(std::mktime(date));
	auto time = std::chrono::duration_cast<std::chrono::seconds>(now - midnight).count();
	return static_cast<float>(time) / 86400.f;
}
