#ifndef __APP_HPP__
#define __APP_HPP__

#include <map>
#include <mutex>
#include <stack>
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

constexpr unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

constexpr float CAM_DIST = 1.6f;
constexpr float SUN_DIST = 1000;

class app {
public:
	GLFWwindow *w;

	app(int ww, int wh, const char *title);
	~app();

	void mainloop();
	void no_event_mainloop();
	void click(int btn, int act, int mod);
	void scroll(int x, int y);
	void key(int key, int act, int mod);
	void write(unsigned int c);
	void resize(int ww, int wh);
	void recv(const packet &p);
	void connect();
	void tbgameWrite();
	void writeChat(const std::string &str);
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
	mesh arrow;
	bool showtrail;
	size_t traillen;
	glw::vao trail;
	glw::vbo trailvbo;
	
	glw::shader postsh;
	glw::shader sh3d;
	glw::shader lightsh;
	glw::shader blursh;
	glw::shader trgsh;
	glw::shader trailsh;

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
	glw::fbo clickfbo;
	glw::tex2 clicktex;

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
	size_t bestPath;
	std::vector<float> showtraildata;
	bool showtraildatadirty;
	std::mutex showtraildatalock;
	std::stack<std::pair<colors::color_t, glm::ivec2>> movestack;
	bool showing;
	int selectedbot;
	
	gamestage stg;

	glgui::container gui;
	glgui::label lbtitle;
	glgui::textbox tbnick;
	glgui::textbox tbip;
	glgui::textbox tbport;
	glgui::button btnconnect;
	glgui::container ingamegui;
	glgui::label lbchat;
	glgui::textbox tbgame;
	glgui::label lbbestpath;

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
	void renderTrail();
};

#endif
