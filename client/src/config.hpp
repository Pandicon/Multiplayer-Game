#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

#include <fstream>
#include <iostream>
#include <string>
#include <json.hpp>

struct serverinfo {
	std::string name;
	std::string ip;
	std::string port;
};
class config {
public:
	float sensitivity;
	float scrollsensitivity;
	std::string grMode;
	int sleepms;
	float exposure;

	size_t maxChatLines;

	std::string defaultNick;
	serverinfo defaultserv;

	bool shadows;
	int shadowSize;
	size_t bloomPasses;
	size_t menuBlur;
	bool antialias;
	size_t antialiasSamples;

	inline void load() {
		std::cout << "[Confiurations]: loading config" << std::endl;
		std::ifstream fglobal("./settings.json");
		if (!fglobal.good()) {
			std::cout << "[Confiurations]: failed opening settings.json" << std::endl;
		}
		nlohmann::json j = nlohmann::json::parse(fglobal);
		fglobal.close();
		sensitivity = j["sensitivity"];
		scrollsensitivity = j["scrollSensitivity"];
		grMode = j["graphics"];
		sleepms = 1000 / static_cast<int>(j["maxfps"]);
		exposure = j["exposure"];

		maxChatLines = j["chat"]["maxlines"];

		defaultNick = j["defaultNickname"];
		parseServerInfo(defaultserv, j["defaultServer"]);

		std::ifstream fgraphics("./configs/graphics-settings.json");
		if (!fgraphics.good()) {
			std::cout << "[Confiurations]: failed opening ./configs/graphics-settings.json" << std::endl;
		}
		nlohmann::json jg = nlohmann::json::parse(fgraphics);
		fgraphics.close();
		shadows = jg[grMode]["shadows"];
		shadowSize = jg[grMode]["shadowResolution"];
		bloomPasses = jg[grMode]["bloomBlurPasses"];
		menuBlur = jg[grMode]["menuBlur"];
		antialias = jg[grMode]["antialiasing"];
		antialiasSamples = jg[grMode]["antialiasSamples"];
	}
private:
	inline void parseServerInfo(serverinfo &si, const nlohmann::json &j) {
		si.name = j["name"];
		si.ip = j["ip"];
		unsigned int port = j["port"];
		si.port = std::to_string(port);
	}
};

#endif
