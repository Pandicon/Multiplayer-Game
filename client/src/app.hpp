#ifndef __APP_HPP__
#define __APP_HPP__

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "glw.hpp"
#include "mesh.hpp"
#include "networking/client.hpp"

class app {
public:
	GLFWwindow *w;

	app(int ww, int wh, const char *title);
	~app();

	void mainloop();
	void no_event_mainloop();
	void resize(int ww, int wh);
	void recv(const packet &p);
private:
	float dt;
	int ww, wh;

	client cl;

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
	glw::shader blursh;

	glw::tex2_array<2> boardtex;
	glw::tex2_array<2> walltex;
	glw::tex2 whitetex;
	glw::tex2 blacktex;

	glw::fbo postfbo;
	glw::tex2 posttex;
	glw::tex2 posttexover;
	glw::tex2 postdepth;
	glw::fbo tmpfbo;
	glw::tex2 tmptex;
	glw::fbo tmp2fbo;
	glw::tex2 tmp2tex;
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

	void setSun();
	void initRendering();
	void update();
	void render();
	void tick();
	void renderScene(const glm::mat4 &vp, glw::shader &sh);
};

#endif
