#ifndef __APP_HPP__
#define __APP_HPP__

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "glw.hpp"

class app {
public:
	GLFWwindow *w;

	app(int ww, int wh, const char *title);
	void mainloop();
	void no_event_mainloop();
	void resize(int ww, int wh);
private:
	float dt;

	glw::vao quad;
	glw::vbo quadvbo;
	glw::ebo quadebo;
	glw::vao board;
	glw::vbo boardvbo;
	glw::ebo boardebo;
	
	glw::shader postsh;
	glw::shader sh3d;

	glw::tex2 boardtex;
	glw::tex2 boardspectex;

	glm::mat4 proj;
	glm::vec2 camorient;
	glm::vec2 prevm;
	glm::vec3 sunpos;
	glm::vec3 skycol;
	glm::vec3 lamppos;
	bool lamp;

	void tick();
};

#endif
