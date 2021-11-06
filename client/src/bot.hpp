#ifndef __BOT_HPP__
#define __BOT_HPP__

#include <inttypes.h>
#include <glm/glm.hpp>

namespace colors {
	enum color_t : uint8_t {
		RED,
		GREEN,
		BLUE,
		YELLOW,
		GRAY
	};
	constexpr glm::vec3 toRGB[] = {
		glm::vec3(1.f, .1f, .2f),
		glm::vec3(.2f, .7f, .1f),
		glm::vec3(.1f, .1f, 1.f),
		glm::vec3(.8f, .8f, .0f),
		glm::vec3(.5f, .5f, .5f)
	};
}

class bot {
public:
	colors::color_t color;
	glm::ivec2 pos;
	glm::ivec2 startpos;
};

#endif
