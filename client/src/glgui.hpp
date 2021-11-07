#ifndef __GLGUI_HPP__
#define __GLGUI_HPP__

#include <inttypes.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
	extern glw::shader guish;
	extern glw::shader guioutlsh;

	template<typename F1, typename F2>
	inline void init(const std::string &shadersdir, const std::string &fonttex, F1 terr, F2 sherr) {
		glw::compileShaderFromFile(guish, (shadersdir + "/gui").c_str(), sherr);
		guish.use();
		guish.uniform1i("tex", 0);
		guish.uniform1i("usetex", 1);
		glw::compileShaderFromFile(guioutlsh,
			(shadersdir + "/gui.vert").c_str(),
			(shadersdir + "/guioutl.frag").c_str(), sherr);
		guioutlsh.use();
		guioutlsh.uniform1i("tex", 0);
		font.fromFile(fonttex, terr, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_NEAREST, GL_RGBA, GL_RGBA8);
	}

	class control {
	public:
		glm::ivec2 pos;
		glm::ivec2 size;
		anchor::anchor_t anch;
		anchor::anchor_t align;
		unsigned int focused;
		
		inline virtual ~control() { }
		inline virtual void init() { }
		inline virtual void mousedown(int mb, int mx, int my) { (void)mb;(void)mx;(void)my; }
		inline virtual void mouseup(int mb, int mx, int my) { (void)mb;(void)mx;(void)my; }
		inline virtual void keydown(int key) { (void)key; }
		inline virtual void keywrite(unsigned int ch) { (void)ch; }
		inline virtual void keyup(int key) { (void)key; }
		inline virtual void update(float dt) { (void)dt; }
		virtual void render(const glm::mat4 &proj) const = 0;
		inline virtual void resize() { }
	};
	class container : public control {
	public:
		std::vector<control *> controls;

		void init() override;
		void mousedown(int mb, int mx, int my) override;
		void mouseup(int mb, int mx, int my) override;
		void keydown(int key) override;
		void keywrite(unsigned int ch) override;
		void keyup(int key) override;
		void update(float dt) override;
		void render(const glm::mat4 &proj) const override;
	};
	class label : public control {
	public:
		bool autosize = true;
		glm::ivec2 charsize;
		bool outline = false;
		glm::vec3 color;

		void render(const glm::mat4 &proj) const override;
		std::string text() const;
		void setText(const std::string &t);
	protected:
		std::string txt;
	};
};

#endif

#ifdef GLGUI_IMPL

glw::high::tiledTextureAtlas<16, 16> glgui::font;
glw::shader glgui::guish;
glw::shader glgui::guioutlsh;

void glgui::container::init() {
	for (auto *c : controls)
		c->init();
}
void glgui::container::mousedown(int mb, int mx, int my) {
	for (auto *c : controls) {
		glm::ivec2 rpos = glm::ivec2(mx, my) - c->pos;
		if (rpos.x > 0 && rpos.y > 0 && rpos.x < c->size.x && rpos.y < c->size.y) {
			c->mousedown(mb, rpos.x, rpos.y);
		}
	}
}
void glgui::container::mouseup(int mb, int mx, int my) {
	for (auto *c : controls) {
		glm::ivec2 rpos = glm::ivec2(mx, my) - c->pos;
		if (rpos.x > 0 && rpos.y > 0 && rpos.x < c->size.x && rpos.y < c->size.y) {
			c->mouseup(mb, rpos.x, rpos.y);
		}
	}
}
void glgui::container::keydown(int key) { (void)key; }
void glgui::container::keywrite(unsigned int ch) { (void)ch; }
void glgui::container::keyup(int key) { (void)key; }
void glgui::container::update(float dt) {
	for (auto *c : controls) {
		c->update(dt);
	}
}
void glgui::container::render(const glm::mat4 &proj) const {
	glm::vec3 transl = -glm::vec3(align % 3 * .5f * size.x, align / 3 * .5f * size.y, 0);
	transl += glm::fvec3(pos, 0);
	for (auto *c : controls) {
		glm::vec3 trloc = glm::vec3(c->anch % 3 * .5f * size.x, c->anch / 3 * .5f * size.y, 0);
		glm::mat4 m = glm::translate(proj, transl + trloc);
		c->render(m);
	}
}
void glgui::label::render(const glm::mat4 &proj) const {
	glm::vec3 transl = -glm::vec3(align % 3 * .5f * size.x, align / 3 * .5f * size.y, 0);
	transl += glm::fvec3(pos, 0);
	glm::mat4 m = glm::translate(proj, transl);
	if (outline) {
		glgui::guioutlsh.use();
		glgui::guioutlsh.uniformM4f("proj", m);
		glgui::guioutlsh.uniform3f("col", color);
	} else {
		glgui::guish.use();
		glgui::guish.uniformM4f("proj", m);
		glgui::guish.uniform3f("col", color);
	}
	glw::high::rendStr(txt, 0, 0, charsize.x, charsize.y, 0, glgui::font);
}
std::string glgui::label::text() const { return txt; }
void glgui::label::setText(const std::string &t) {
	txt = t;
	if (autosize) {
		size_t w = 0, h = 1, c = 0;
		for (auto it = txt.begin(); it != txt.end(); ++it, ++c) {
			if (*it == '\n') {
				++h;
				if (c > w)
					w = c;
				c = 0;
			}
		}
		if (c > w)
			w = c;
		size = charsize * glm::ivec2(w, h);
	}
}

/*
void glgui::label::init() {
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
void glgui::label::render() const {
	glgui::guish.use();
	glgui::guish.uniformM4f("proj", glgui::projs[anch]);
	glgui::guish.uniform3f("col", color);
	glw::high::rendStr(text, pos.x - off.x, pos.y - off.y, charsize.x, charsize.y, 0, glgui::font);
}
void glgui::outlinedlabel::render() const {
	glgui::guioutlsh.use();
	glgui::guioutlsh.uniformM4f("proj", glgui::projs[anch]);
	glgui::guioutlsh.uniform3f("col", color);
	glw::high::rendStr(text, pos.x - off.x, pos.y - off.y, charsize.x, charsize.y, 0, glgui::font);
}
void glgui::button::init() {
	off = glm::fvec2(size) * glm::vec2(align % 3 * .5f, align / 3 * .5f);
	glm::vec2 st = glm::fvec2(pos) - off;
	glm::vec2 ed = st + glm::fvec2(size);
	float v[] = {
		st.x, st.y, 0, 0,
		st.x, ed.y, 0, 1,
		ed.x, ed.y, 1, 1,
		ed.x, st.y, 1, 0
	};
	initVaoVbo(a, b, v, sizeof(float) * 16,
		sizeof(float) * 4, {glw::vap(2),glw::vap(2,sizeof(float)*2)}, GL_STREAM_DRAW);
	size_t w = 0, h = 1, c = 0;
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
}
void glgui::button::render() const {
	glgui::guish.use();
	glgui::guish.uniformM4f("proj", glgui::projs[anch]);
	glgui::guish.uniform3f("col", bgcolor);
	glgui::guish.uniform1i("usetex", 0);
	a.bind();
	a.drawArrays(4, GL_TRIANGLE_FAN);
	glgui::guish.uniform1i("usetex", 1);
}*/

#endif
