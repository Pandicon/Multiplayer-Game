#ifndef __GLGUI_HPP__
#define __GLGUI_HPP__

#include <inttypes.h>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
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
	extern glw::vao quad;
	extern glw::vbo quadb;

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
		float quadverts[] = {
			0, 0, 0, 0,
			1, 0, 1, 0,
			1, 1, 1, 1,
			0, 1, 0, 1
		};
		glw::initVaoVbo(quad, quadb, quadverts, sizeof(float)*16, sizeof(float)*4,
			{glw::vap(2),glw::vap(2,sizeof(float)*2)}, GL_STATIC_DRAW);
	}

	class control {
	public:
		glm::fvec2 pos;
		glm::fvec2 size;
		anchor::anchor_t anch;
		anchor::anchor_t align;
		bool focused;
		
		inline virtual ~control() { }
		inline virtual void init() { }
		inline virtual void mousedown(int mb, float mx, float my) { (void)mb;(void)mx;(void)my; }
		inline virtual void mouseup(int mb, float mx, float my) { (void)mb;(void)mx;(void)my; }
		inline virtual void keydown(int key) { (void)key; }
		inline virtual void keywrite(char ch) { (void)ch; }
		inline virtual void keyup(int key) { (void)key; }
		inline virtual void update(float dt) { (void)dt; }
		virtual void render(const glm::mat4 &proj) const = 0;
		inline virtual void resize() { }
		inline virtual void unfocus() { focused = false; }
	};
	class container : public control {
	public:
		std::vector<control *> controls;

		void init() override;
		void mousedown(int mb, float mx, float my) override;
		void mouseup(int mb, float mx, float my) override;
		void keydown(int key) override;
		void keywrite(char ch) override;
		void keyup(int key) override;
		void update(float dt) override;
		void render(const glm::mat4 &proj) const override;
		void unfocus() override;
	};
	class label : public control {
	public:
		bool autosize = true;
		glm::fvec2 charsize;
		bool outline = false;
		glm::vec3 color;

		void render(const glm::mat4 &proj) const override;
		std::string text() const;
		void setText(const std::string &t);
	protected:
		std::string txt;
	};
	using glgui_callback = void (*)(void *);
	class button : public control {
	public:
		glm::fvec2 charsize;
		glm::vec3 bgcolor;
		anchor::anchor_t textalign;
		void *data;
		glgui_callback cb = nullptr;

		void mousedown(int mb, float mx, float my) override;
		void mouseup(int mb, float mx, float my) override;
		void keydown(int key) override;
		void render(const glm::mat4 &proj) const override;
		std::string text() const;
		void setText(const std::string &t);
	protected:
		std::string txt;
		glm::vec2 textsize;
	};
	class textbox : public control {
	public:
		glm::fvec2 charsize;
		unsigned int cursorpos;
		void *data;
		glgui_callback cb = nullptr;
		std::string text;

		void mousedown(int mb, float mx, float my) override;
		void keydown(int key) override;
		void keywrite(char ch) override;
		void render(const glm::mat4 &proj) const override;
	};
}

#endif

#ifdef GLGUI_IMPL

glw::high::tiledTextureAtlas<16, 16> glgui::font;
glw::shader glgui::guish;
glw::shader glgui::guioutlsh;
glw::vao glgui::quad;
glw::vbo glgui::quadb;

glm::ivec2 textdims(const std::string &s) {
	size_t w = 0, h = 1, c = 0;
	for (auto it = s.begin(); it != s.end(); ++it, ++c) {
		if (*it == '\n') {
			++h;
			if (c > w)
				w = c;
			c = 0;
		}
	}
	if (c > w)
		w = c;
	return glm::ivec2(w, h);
}

void glgui::container::init() {
	for (auto *c : controls)
		c->init();
}
void glgui::container::mousedown(int mb, float mx, float my) {
	for (auto *c : controls) {
		glm::vec2 transl(c->anch % 3 * .5f * size.x, c->anch / 3 * .5f * size.y);
		transl -= glm::vec2(c->align % 3 * .5f * c->size.x, c->align / 3 * .5f * c->size.y);
		transl += c->pos;
		glm::vec2 rpos = glm::vec2(mx, my) - transl;
		if (rpos.x > 0 && rpos.y > 0 && rpos.x < c->size.x && rpos.y < c->size.y) {
			c->mousedown(mb, rpos.x, rpos.y);
		}
	}
}
void glgui::container::mouseup(int mb, float mx, float my) {
	for (auto *c : controls) {
		glm::vec2 transl(c->anch % 3 * .5f * size.x, c->anch / 3 * .5f * size.y);
		transl -= glm::vec2(c->align % 3 * .5f * c->size.x, c->align / 3 * .5f * c->size.y);
		transl += c->pos;
		glm::vec2 rpos = glm::vec2(mx, my) - transl;
		if (rpos.x > 0 && rpos.y > 0 && rpos.x < c->size.x && rpos.y < c->size.y) {
			c->mouseup(mb, rpos.x, rpos.y);
		}
	}
}
void glgui::container::keydown(int key) {
	for (auto *c : controls) {
		if (c->focused) {
			c->keydown(key);
		}
	}
}
void glgui::container::keywrite(char ch) {
	for (auto *c : controls) {
		if (c->focused) {
			c->keywrite(ch);
		}
	}
}
void glgui::container::keyup(int key) {
	for (auto *c : controls) {
		if (c->focused) {
			c->keyup(key);
		}
	}
}
void glgui::container::update(float dt) {
	for (auto *c : controls) {
		c->update(dt);
		if (c->focused) {
			focused = true;
		}
	}
}
void glgui::container::render(const glm::mat4 &proj) const {
	for (auto *c : controls) {
		glm::vec3 transl(c->anch % 3 * .5f * size.x, c->anch / 3 * .5f * size.y, 0);
		transl -= glm::vec3(c->align % 3 * .5f * c->size.x, c->align / 3 * .5f * c->size.y, 0);
		transl += glm::vec3(c->pos, 0);
		glm::mat4 m = glm::translate(proj, transl);
		c->render(m);
	}
}
void glgui::container::unfocus() {
	control::unfocus();
	for (auto *c : controls) {
		c->unfocus();
	}
}
void glgui::label::render(const glm::mat4 &proj) const {
	if (outline) {
		glgui::guioutlsh.use();
		glgui::guioutlsh.uniformM4f("proj", proj);
		glgui::guioutlsh.uniform3f("col", color);
	} else {
		glgui::guish.use();
		glgui::guish.uniformM4f("proj", proj);
		glgui::guish.uniform3f("col", color);
	}
	glw::high::rendStr(txt, 0, 0, charsize.x, charsize.y, 0, glgui::font);
}
std::string glgui::label::text() const { return txt; }
void glgui::label::setText(const std::string &t) {
	txt = t;
	if (autosize) {
		size = charsize * glm::fvec2(textdims(txt));
	}
}
void glgui::button::mousedown(int mb, float mx, float my) {
	(void)mx; (void)my;
	if (mb == GLFW_MOUSE_BUTTON_LEFT)
		focused = true;
}
void glgui::button::mouseup(int mb, float mx, float my) {
	(void)mx; (void)my;
	if (mb == GLFW_MOUSE_BUTTON_LEFT && cb != nullptr)
		cb(data);
}
void glgui::button::keydown(int key) {
	if ((key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE) && cb != nullptr)
		cb(data);
}
void glgui::button::render(const glm::mat4 &proj) const {
	glm::mat4 m = glm::scale(proj, glm::vec3(size, 1.f));
	glm::vec3 transl = glm::vec3(textalign % 3 * .5f * size.x, textalign / 3 * .5f * size.y, 0);
	transl += -glm::vec3(textalign % 3 * .5f * textsize.x, textalign / 3 * .5f * textsize.y, 0);
	glm::mat4 m2 = glm::translate(proj, transl);
	glgui::guish.use();
	glgui::guish.uniformM4f("proj", m);
	glgui::guish.uniform3f("col", bgcolor);
	glgui::guish.uniform1i("usetex", 0);
	glgui::quad.bind();
	glgui::quad.drawArrays(4, GL_TRIANGLE_FAN);
	glgui::guish.uniform1i("usetex", 1);
	glgui::guish.uniformM4f("proj", m2);
	glgui::guish.uniform3f("col", 0, 0, 0);
	glw::high::rendStr(txt, 0, 0,
		charsize.x,
		charsize.y, 0, glgui::font);
}
std::string glgui::button::text() const { return txt; }
void glgui::button::setText(const std::string &t) {
	txt = t;
	textsize = charsize * glm::fvec2(textdims(txt));
}

void glgui::textbox::mousedown(int mb, float mx, float my) {
	(void)mb;
	(void)my;
	if (mb == GLFW_MOUSE_BUTTON_LEFT) {
		focused = true;
		cursorpos = static_cast<int>(mx / charsize.x);
		if (cursorpos > text.size())
			cursorpos = text.size();
	}
}
void glgui::textbox::keydown(int key) {
	switch (key) {
	case GLFW_KEY_LEFT:
		if (cursorpos > 0)
			--cursorpos;
		break;
	case GLFW_KEY_RIGHT:
		if (cursorpos < text.size())
			++cursorpos;
		break;
	case GLFW_KEY_DELETE:
		if (cursorpos < text.size())
			text.erase(text.begin()+cursorpos);
		break;
	case GLFW_KEY_BACKSPACE:
		if (cursorpos > 0)
			text.erase(text.begin()+(--cursorpos));
		break;
	case GLFW_KEY_ENTER:
		if (cb != nullptr)
			cb(data);
		break;
	default:
		break;
	}
}
void glgui::textbox::keywrite(char ch) {
	text.insert(cursorpos++, 1, ch);
}
void glgui::textbox::render(const glm::mat4 &proj) const {
	glm::mat4 m = glm::scale(proj, glm::vec3(size, 1.f));
	glm::mat4 m2 = glm::translate(proj, glm::vec3(0, .5f * size.y - .5f * charsize.y, 0));
	glm::mat4 m3 = glm::translate(proj, glm::vec3(charsize.x * cursorpos, .5f * size.y - .5f * charsize.y, 0));
	m3 = glm::scale(m3, glm::vec3(3.f, charsize.y, 1.f));
	glgui::guish.use();
	glgui::guish.uniformM4f("proj", m);
	glgui::guish.uniform3f("col", 1, 1, 1);
	glgui::guish.uniform1i("usetex", 0);
	glgui::quad.bind();
	glgui::quad.drawArrays(4, GL_TRIANGLE_FAN);
	glgui::guish.uniform3f("col", 0, 0, 0);
	if (focused) {
		glgui::guish.uniformM4f("proj", m3);
		glgui::quad.drawArrays(4, GL_TRIANGLE_FAN);
	}
	glgui::guish.uniform1i("usetex", 1);
	glgui::guish.uniformM4f("proj", m2);
	glw::high::rendStr(text, 0, 0,
		charsize.x,
		charsize.y, 0, glgui::font);
}

#endif
