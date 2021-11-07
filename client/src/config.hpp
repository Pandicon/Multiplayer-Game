#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <fstream>
#include <iostream>
#include <string>
#include <json.hpp>

class config {
public:
	float sensitivity;
	std::string grMode;
	int sleepms;
	bool shadows;
	int shadowSize;
	size_t bloomPasses;
	float exposure;

	inline void load() {
		std::ifstream fglobal("./settings.json");
		if (!fglobal.good()) {
			std::cout << "[Confiurations]: failed opening settings.json" << std::endl;
		}
		nlohmann::json j = nlohmann::json::parse(fglobal);
		fglobal.close();
		sensitivity = j["sensitivity"];
		grMode = j["graphics"];
		sleepms = 1000 / static_cast<int>(j["maxfps"]);
		exposure = j["exposure"];
		std::ifstream fgraphics("./configs/graphics-settings.json");
		if (!fgraphics.good()) {
			std::cout << "[Confiurations]: failed opening ./configs/graphics-settings.json" << std::endl;
		}
		nlohmann::json jg = nlohmann::json::parse(fgraphics);
		fgraphics.close();
		shadows = jg[grMode]["shadows"];
		shadowSize = jg[grMode]["shadowResolution"];
		bloomPasses = jg[grMode]["bloomBlurPasses"];
	}
};

#endif
