#ifndef __APP_HPP__
#define __APP_HPP__

#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "board.hpp"
#include "bot.hpp"
#include "config.hpp"
#include "glgui.hpp"
#include "glw.hpp"
#include "mesh.hpp"
#include "networking/client.hpp"

enum class gamestage {
	MENU,
	IN_GAME
};

class app {
public:
	GLFWwindow *w;

	app(int ww, int wh, const char *title);
	~app();

	void mainloop();
	void no_event_mainloop();
	void click(int btn, int act, int mod);
	void resize(int ww, int wh);
	void recv(const packet &p);
	void connect();
private:
	float dt;
	int ww, wh;
	float fov;

	config cfg;

	client cl;

	glw::vao quad;
	glw::vbo quadvbo;
	glw::ebo quadebo;
	mesh boardmesh;
	mesh wall;
	mesh robot;
	mesh sun;
	
	glw::shader postsh;
	glw::shader sh3d;
	glw::shader lightsh;
	glw::shader blursh;
	glw::shader trgsh;

	glw::tex2_array<2> boardtex;
	glw::tex2_array<2> walltex;
	glw::tex2 whitetex;
	glw::tex2 blacktex;

	glw::fbo postfbo;
	glw::tex2 posttex;
	glw::tex2 posttexover;
	glw::tex2 postdepth;
	glw::fbo postfboms;
	glw::tex2multisample posttexms;
	glw::tex2multisample posttexoverms;
	glw::tex2multisample postdepthms;
	glw::fbo postfbomscolor0;
	glw::fbo postfbomscolor1;
	glw::fbo postfbocolor0;
	glw::fbo postfbocolor1;
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
	float sunstrength;

	board brd;
	bot bots[5];
	target trg;
	
	gamestage stg;

	glgui::container gui;
	glgui::label lbtitle;
	glgui::button btnconnect;

	void setSun();
	void initRendering();
	void initModels();
	void initShaders();
	void initTextures();
	void initFramebuffers();
	void initGUI();
	void update();
	void render();
	void renderGame();
	void tick();
	void renderScene(const glm::mat4 &vp, glw::shader &sh);
	void renderTarget();
};

#endif
