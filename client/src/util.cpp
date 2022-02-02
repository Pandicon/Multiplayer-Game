#include "util.hpp"

#include <chrono>
#include <iostream>
#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>

float getDayProgress() {
	auto now = std::chrono::system_clock::now();
	time_t tnow = std::chrono::system_clock::to_time_t(now);
	tm *date = std::localtime(&tnow);
	date->tm_hour = 0;
	date->tm_min = 0;
	date->tm_sec = 0;
	auto midnight = std::chrono::system_clock::from_time_t(std::mktime(date));
	auto time = std::chrono::duration_cast<std::chrono::seconds>(now - midnight).count();
	float rawDayProgress = static_cast<float>(time) / 86400.f;
	int yearDay = date->tm_yday;
	constexpr int shortestDayOffset = 9;
	constexpr float lat = 0.87266f;
	float yearProgress = (yearDay - 1 + rawDayProgress + shortestDayOffset) / 365.f;
	float decl = glm::cos(yearProgress * glm::pi<float>()) * 0.41015237421f;
	float sunrise = glm::acos(-glm::tan(lat)*glm::tan(decl)) / (2.f * glm::pi<float>());
	float sunset = glm::acos(glm::tan(lat)*glm::tan(decl)) / (2.f * glm::pi<float>()) + .5f;
	int sunriseMin = static_cast<int>(sunrise * 1440.f);
	int sunsetMin = static_cast<int>(sunset * 1440.f);
	std::cout << "[Sun calculations]: sunrise " << (sunriseMin / 60) << ":" << (sunriseMin % 60) << std::endl;
	std::cout << "[Sun calculations]: sunset " << (sunsetMin / 60) << ":" << (sunsetMin % 60) << std::endl;
	constexpr float inSunrise = 0.25f;
	constexpr float inSunset = 0.75f;
	constexpr float outSunrise = 0.f;
	constexpr float outSunset = 0.5f;
	float dayProgress = (rawDayProgress - inSunrise) * (sunset - sunrise) / (inSunset - inSunrise)/* + sunrise*/;
	dayProgress = (dayProgress/* - sunrise*/) * (outSunset - outSunrise) / (sunset - sunrise) + outSunrise;
	return glm::clamp(dayProgress, -1.f, 1.f);
}
