#ifndef __APP_HPP__
#define __APP_HPP__

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "glw.hpp"
#include "mesh.hpp"

class app {
public:
	GLFWwindow *w;

	app(int ww, int wh, const char *title);
	void mainloop();
	void no_event_mainloop();
	void resize(int ww, int wh);
private:
	float dt;
	int ww, wh;

	glw::vao quad;
	glw::vbo quadvbo;
	glw::ebo quadebo;
	glw::vao board;
	glw::vbo boardvbo;
	glw::ebo boardebo;
	glw::vao wall;
	glw::vbo wallvbo;
	glw::ebo wallebo;
	mesh robot;
	
	glw::shader postsh;
	glw::shader sh3d;
	glw::shader lightsh;

	glw::tex2_array<2> boardtex;
	glw::tex2_array<2> walltex;
	glw::tex2 whitetex;
	glw::tex2 blacktex;

	glw::fbo postfbo;
	glw::tex2 posttex;
	glw::tex2 postdepth;
	glw::fbo sunfbo;
	glw::tex2 sundepth;
	glw::fbo lampfbo;
	glw::tex2 lampdepth;

	glm::mat4 proj;
	glm::vec2 camorient;
	glm::vec2 prevm;
	glm::vec3 sunpos;
	glm::vec3 skycol;
	glm::vec3 lamppos;
	bool lamp;

	void tick();
	void render(const glm::mat4 &vp, glw::shader &sh);
};

#endif
