#ifndef __GLGUI_HPP__
#define __GLGUI_HPP__

#include <inttypes.h>
#include <string>
#include <glm/glm.hpp>
#include "glw.hpp"

namespace glgui {
	namespace anchor {
		enum anchor_t : uint8_t {
			TOPLEFT, TOPMID, TOPRIGHT,
			MIDLEFT, CENTER, MIDRIGHT,
			BOTLEFT, BOTMID, BOTRIGHT,
			COUNT
		};
	}

	extern glw::high::tiledTextureAtlas<16, 16> font;
	extern glm::mat4 projs[anchor::COUNT];
	extern glw::shader guish;
	extern glw::shader guioutlsh;

	template<typename F1, typename F2>
	inline void init(const std::string &shadersdir, const std::string &fonttex, F1 terr, F2 sherr) {
		glw::compileShaderFromFile(guish, (shadersdir + "/gui").c_str(), sherr);
		guish.use();
		guish.uniform1i("tex", 0);
		glw::compileShaderFromFile(guioutlsh,
			(shadersdir + "/gui.vert").c_str(),
			(shadersdir + "/guioutl.frag").c_str(), sherr);
		guioutlsh.use();
		guioutlsh.uniform1i("tex", 0);
		font.fromFile(fonttex, terr, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_NEAREST, GL_RGBA, GL_RGBA8);
	}
	void updateproj(int ww, int wh);

	class label {
	public:
		glm::ivec2 pos;
		anchor::anchor_t anch;
		anchor::anchor_t align;
		glm::ivec2 charsize;
		glm::vec3 color;

		void render() const;
		void setText(const std::string &t);
	protected:
		std::string text;
		glm::vec2 off;
	};
	class outlinedlabel : public label {
	public:
		void render() const;
	private:
	};
};

#endif

#ifdef GLGUI_IMPL

glw::high::tiledTextureAtlas<16, 16> glgui::font;
glm::mat4 glgui::projs[anchor::COUNT];
glw::shader glgui::guish;
glw::shader glgui::guioutlsh;

void glgui::updateproj(int ww, int wh) {
	float hw = ww * .5f, hh = wh * .5f;
	glgui::projs[anchor::TOPLEFT] =  glm::ortho<float>(0.f, ww,  wh,  0.f);
	glgui::projs[anchor::TOPMID] =   glm::ortho<float>(-hw, hw,  wh,  0.f);
	glgui::projs[anchor::TOPRIGHT] = glm::ortho<float>(-ww, 0.f, wh,  0.f);
	glgui::projs[anchor::MIDLEFT] =  glm::ortho<float>(0.f, ww,  -hh, hh);
	glgui::projs[anchor::CENTER] =   glm::ortho<float>(-hw, hw,  -hh, hh);
	glgui::projs[anchor::MIDRIGHT] = glm::ortho<float>(-ww, 0.f, -hh, hh);
	glgui::projs[anchor::BOTLEFT] =  glm::ortho<float>(0.f, ww,  0.f, wh);
	glgui::projs[anchor::BOTMID] =   glm::ortho<float>(-hw, hw,  0.f, wh);
	glgui::projs[anchor::BOTRIGHT] = glm::ortho<float>(-ww, 0.f, 0.f, wh);
}
void glgui::label::render() const {
	glgui::guish.use();
	glgui::guish.uniformM4f("proj", glgui::projs[anch]);
	glgui::guish.uniform3f("col", color);
	glw::high::rendStr(text, pos.x - off.x, pos.y - off.y, charsize.x, charsize.y, 0, glgui::font);
}
void glgui::label::setText(const std::string &t) {
	text = t;
	size_t w = 0, h = 0, c = 0;
	for (auto it = text.begin(); it != text.end(); ++it, ++c) {
		if (*it == '\n') {
			++h;
			if (c > w)
				w = c;
			c = 0;
		}
	}
	if (c > w)
		w = c;
	off = glm::vec2(charsize.x * w, charsize.y * h) * glm::vec2(align % 3 * .5f, align / 3 * .5f);
}
void glgui::outlinedlabel::render() const {
	glgui::guioutlsh.use();
	glgui::guioutlsh.uniformM4f("proj", glgui::projs[anch]);
	glgui::guioutlsh.uniform3f("col", color);
	glw::high::rendStr(text, pos.x - off.x, pos.y - off.y, charsize.x, charsize.y, 0, glgui::font);
}

#endif
