#include "app.hpp"

#include <math.h>
#include <bitset>
#include <chrono>
#include <iostream>
#include <thread>
#include <glm/gtc/matrix_transform.hpp>
#include "config.hpp"

unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

constexpr float CAM_DIST = 1.6f;
constexpr float SUN_DIST = 1000;

void onRecv(packet &p, void *data) {
	((app *)data)->recv(p);
}

app::app(int ww, int wh, const char *title) : ww(ww), wh(wh), cl(onRecv, this), camorient(1, 0) {
	w = glfwCreateWindow(ww, wh, title, NULL, NULL);
	glfwMakeContextCurrent(w);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "OpenGL load failed. Does your graphics card support OpenGL 4.0 core?" << std::endl;
	}
	cfg.load();
	fov = 2.f;
	for (size_t i = 0; i < BOARD_TILES; ++i) {
		bool *tile = brd.walls[i % BOARD_SIZE][i / BOARD_SIZE];
		for (uint8_t j = 0; j < 4; ++j)
			tile[j] = false;
	}
	for (uint8_t i = 0; i < 5; ++i) {
		bots[i].color = static_cast<colors::color_t>(i);
		bots[i].pos = glm::ivec2(i, 0);
	}
	trg.color = colors::YELLOW;
	trg.pos = glm::ivec2(3, 1);
	setSun();
	initRendering();
	resize(ww, wh);

	stg = gamestage::MENU;

	//cl.connect("127.0.0.1", "5050");
}
app::~app() {
	if (cl.running) {
		cl.send(packet(packets::C_S_DISCONNECT, nullptr, 0));
		cl.disconnect();
	}
}
void app::mainloop() {
	double prev = glfwGetTime();
	while (!glfwWindowShouldClose(w)) {
		double now = glfwGetTime();
		dt = static_cast<float>(now - prev);
		prev = now;
		tick();
		glfwSwapBuffers(w);
		glfwPollEvents();
		std::this_thread::sleep_for(std::chrono::milliseconds(cfg.sleepms));
	}
}
void app::no_event_mainloop() {
	double prev = glfwGetTime();
	while (!glfwWindowShouldClose(w)) {
		double now = glfwGetTime();
		dt = static_cast<float>(now - prev);
		prev = now;
		tick();
		glfwSwapBuffers(w);
		std::this_thread::sleep_for(std::chrono::milliseconds(cfg.sleepms));
	}
}
void app::resize(int ww, int wh) {
	this->ww = ww;
	this->wh = wh;
	proj = glm::perspective(fov, static_cast<float>(ww) / wh, .1f, 100.f);
	posttex.bind();
	posttex.size = glm::ivec2(ww, wh);
	posttex.upload(NULL, GL_RGBA, GL_RGBA16F, GL_FLOAT);
	posttexover.bind();
	posttexover.size = glm::ivec2(ww, wh);
	posttexover.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	postdepth.bind();
	postdepth.size = glm::ivec2(ww, wh);
	postdepth.upload(NULL, GL_DEPTH_STENCIL, GL_DEPTH24_STENCIL8, GL_UNSIGNED_INT_24_8);
	posttexms.bind();
	posttexms.size = glm::ivec2(ww, wh);
	posttexms.generate(GL_RGBA16F, cfg.antialiasSamples);
	posttexoverms.bind();
	posttexoverms.size = glm::ivec2(ww, wh);
	posttexoverms.generate(GL_RGBA8, cfg.antialiasSamples);
	postdepthms.bind();
	postdepthms.size = glm::ivec2(ww, wh);
	postdepthms.generate(GL_DEPTH24_STENCIL8, cfg.antialiasSamples);
	tmptex.bind();
	tmptex.size = glm::ivec2(ww, wh);
	tmptex.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	tmp2tex.bind();
	tmp2tex.size = glm::ivec2(ww, wh);
	tmp2tex.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	gui.size = glm::ivec2(ww, wh);
	gui.resize();
}
void app::recv(const packet &p) {
	switch (p.type()) {
	case packets::S_C_DISCONNECT:
		std::cout << "Disconnected!" << std::endl;
		break;
	case packets::S_C_WALLS:{
		size_t x = 0, y = 0;
		for (size_t i = 0; i < p.size(); ++i) {
			char dat1(p.data()[i] >> 4 & 0xf);
			char dat2(p.data()[i] & 0xf);
			uint8_t lookup[] = { 1, 0, 3, 2 };
			for (uint8_t j = 0; j < 4; ++j) {
				brd.walls[x][y][lookup[j]] = dat1 & 1;
				brd.walls[x+1][y][lookup[j]] = dat2 & 1;
				dat1 >>= 1;
				dat2 >>= 1;
			}
			x += 2;
			if (i % 8 == 7) {
				++y;
				x = 0;
			}
		}
		break;
	}
	case packets::S_C_ROBOTS:
		// TODO:
		break;
	case packets::S_C_TARGET:
		// TODO:
		break;
	case packets::S_C_MESSAGE:
		std::cout << "[Chat]: " << std::string(p.data(), p.size()) << std::endl;
		break;
	default:
		break;
	}
}

void app::setSun() {
	auto now = std::chrono::system_clock::now();
	time_t tnow = std::chrono::system_clock::to_time_t(now);
    tm *date = std::localtime(&tnow);
    date->tm_hour = 0;
    date->tm_min = 0;
    date->tm_sec = 0;
    auto midnight = std::chrono::system_clock::from_time_t(std::mktime(date));
	auto time = std::chrono::duration_cast<std::chrono::seconds>(now - midnight).count();
	float dayprogress = static_cast<float>(time) / 86400.f;
	sunpos = glm::vec3(sinf(dayprogress*2.f*glm::pi<float>()), -cosf(dayprogress*2.f*glm::pi<float>()), 0.1f);
	sunstrength = sunpos.y < 0 ? 0 : sunpos.y * 2;
	if (sunstrength > 1)
		sunstrength = 1;
	skycol = glm::vec3(.2f, .7f, 1.f) * sunstrength;
	lamp = sunpos.y < 0.1f;
	lamppos = glm::vec3(0.f, 1.0f, 0.001f);
	sunpos *= SUN_DIST;
}
void app::initRendering() {
	glw::checkError("init precheck", glw::justPrint);
	initModels();
	glw::checkError("init models check", glw::justPrint);
	initShaders();
	glw::checkError("init shaders check", glw::justPrint);
	initTextures();
	glw::checkError("init textures check", glw::justPrint);
	initFramebuffers();
	glw::checkError("init framebuffers check", glw::justPrint);
	initGUI();
	glw::checkError("init gui check", glw::justPrint);
}
void app::initModels() {
	float quadverts[] = {
		-1, -1, 0, 0,
		 1, -1, 1, 0,
		 1,  1, 1, 1,
		-1,  1, 0, 1,
	};
	unsigned int quadindices[] = {
		0, 1, 2,
		0, 2, 3
	};
	glw::initVaoVboEbo(quad, quadvbo, quadebo, quadverts, sizeof(quadverts),
		quadindices, sizeof(quadindices), sizeof(float)*4, {glw::vap(2),glw::vap(2, sizeof(float)*2)});
	boardmesh.load("./models/board.obj");
	wall.load("./models/wall.obj");
	robot.load("./models/robot.obj");
	sun.load("./models/sun.obj");
}
void app::initShaders() {
	glw::compileShaderFromFile(postsh, "./shaders/post", glw::default_shader_error_handler());
	postsh.use();
	postsh.uniform1i("tex", 0);
	postsh.uniform1i("bloom", 1);
	glw::compileShaderFromFile(sh3d, "./shaders/s3", glw::default_shader_error_handler());
	sh3d.use();
	sh3d.uniform1i("tex", 0);
	sh3d.uniform1i("texspec", 1);
	sh3d.uniform1i("sundepth", 2);
	sh3d.uniform1i("lampdepth", 3);
	glw::compileShaderFromFile(lightsh, "./shaders/light", glw::default_shader_error_handler());
	glw::compileShaderFromFile(blursh, "./shaders/pass.vert", "./shaders/blur.frag", glw::default_shader_error_handler());
	blursh.use();
	blursh.uniform1i("tex", 0);
	glw::compileShaderFromFile(trgsh, "./shaders/trg", glw::default_shader_error_handler());
}
void app::initTextures() {
	boardtex.gen();
	boardtex[0].bind();
	boardtex[0].setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_LINEAR, GL_LINEAR);
	boardtex[0].fromFile("./textures/board.png", glw::justPrint, "Could not find", GL_RGBA, GL_SRGB_ALPHA);
	boardtex[1].bind();
	boardtex[1].setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_LINEAR, GL_LINEAR);
	boardtex[1].fromFile("./textures/board_spec.png", glw::justPrint, "Could not find", GL_RGBA, GL_RGBA8);
	walltex.gen();
	walltex[0].bind();
	walltex[0].setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_LINEAR, GL_LINEAR);
	walltex[0].fromFile("./textures/wall.png", glw::justPrint, "Could not find", GL_RGBA, GL_SRGB_ALPHA);
	walltex[1].bind();
	walltex[1].setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_LINEAR, GL_LINEAR);
	walltex[1].fromFile("./textures/wall_spec.png", glw::justPrint, "Could not find", GL_RGBA, GL_RGBA8);
	whitetex.gen();
	whitetex.bind();
	whitetex.setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	whitetex.fromFile("./textures/white.png", glw::justPrint, "Could not find", GL_RGBA, GL_SRGB_ALPHA);
	blacktex.gen();
	blacktex.bind();
	blacktex.setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	blacktex.fromFile("./textures/black.png", glw::justPrint, "Could not find", GL_RGBA, GL_SRGB_ALPHA);
}
void app::initFramebuffers() {
	sunfbo.gen();
	sundepth.gen();
	sundepth.bind();
	sundepth.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	sundepth.size = glm::ivec2(cfg.shadowSize, cfg.shadowSize);
	sundepth.upload(NULL, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
	sunfbo.bind();
	sunfbo.attach(sundepth, GL_DEPTH_ATTACHMENT);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	lampfbo.gen();
	lampdepth.gen();
	lampdepth.bind();
	lampdepth.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	lampdepth.size = glm::ivec2(cfg.shadowSize, cfg.shadowSize);
	lampdepth.upload(NULL, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
	lampfbo.bind();
	lampfbo.attach(lampdepth, GL_DEPTH_ATTACHMENT);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	tmpfbo.gen();
	tmptex.gen();
	tmptex.bind();
	tmptex.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	tmptex.size = glm::ivec2(ww, wh);
	tmptex.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	tmpfbo.bind();
	tmpfbo.attach(tmptex, GL_COLOR_ATTACHMENT0);
	tmp2fbo.gen();
	tmp2tex.gen();
	tmp2tex.bind();
	tmp2tex.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	tmp2tex.size = glm::ivec2(ww, wh);
	tmp2tex.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	tmp2fbo.bind();
	tmp2fbo.attach(tmp2tex, GL_COLOR_ATTACHMENT0);
	postfbo.gen();
	posttex.gen();
	posttex.bind();
	posttex.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	posttex.size = glm::ivec2(ww, wh);
	posttex.upload(NULL, GL_RGBA, GL_RGBA16F, GL_FLOAT);
	posttexover.gen();
	posttexover.bind();
	posttexover.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	posttexover.size = glm::ivec2(ww, wh);
	posttexover.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	postdepth.gen();
	postdepth.bind();
	postdepth.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	postdepth.size = glm::ivec2(ww, wh);
	postdepth.upload(NULL, GL_DEPTH_STENCIL, GL_DEPTH24_STENCIL8, GL_UNSIGNED_INT_24_8);
	postfbo.bind();
	postfbo.attach(posttex, GL_COLOR_ATTACHMENT0);
	postfbo.attach(posttexover, GL_COLOR_ATTACHMENT1);
	postfbo.attach(postdepth, GL_DEPTH_STENCIL_ATTACHMENT);
	glDrawBuffers(2, attachments);
	postfboms.gen();
	posttexms.gen();
	posttexms.bind();
	posttexms.size = glm::ivec2(ww, wh);
	posttexms.generate(GL_RGBA16F, cfg.antialiasSamples);
	posttexoverms.gen();
	posttexoverms.bind();
	posttexoverms.size = glm::ivec2(ww, wh);
	posttexoverms.generate(GL_RGBA8, cfg.antialiasSamples);
	postdepthms.gen();
	postdepthms.bind();
	postdepthms.size = glm::ivec2(ww, wh);
	postdepthms.generate(GL_DEPTH24_STENCIL8, cfg.antialiasSamples);
	postfboms.bind();
	postfboms.attach(posttexms, GL_COLOR_ATTACHMENT0);
	postfboms.attach(posttexoverms, GL_COLOR_ATTACHMENT1);
	postfboms.attach(postdepthms, GL_DEPTH_STENCIL_ATTACHMENT);
	glDrawBuffers(2, attachments);
	postfbocolor0.gen();
	postfbocolor0.bind();
	postfbocolor0.attach(posttex, GL_COLOR_ATTACHMENT0);
	postfbocolor0.attach(postdepth, GL_DEPTH_STENCIL_ATTACHMENT);
	postfbocolor1.gen();
	postfbocolor1.bind();
	postfbocolor1.attach(posttexover, GL_COLOR_ATTACHMENT0);
	postfbomscolor0.gen();
	postfbomscolor0.bind();
	postfbomscolor0.attach(posttexms, GL_COLOR_ATTACHMENT0);
	postfbomscolor0.attach(postdepthms, GL_DEPTH_STENCIL_ATTACHMENT);
	postfbomscolor1.gen();
	postfbomscolor1.bind();
	postfbomscolor1.attach(posttexoverms, GL_COLOR_ATTACHMENT0);
	glw::fbo::screen.bind();
}
void app::initGUI() {
	glgui::init("./shaders", "./textures/font.png", glw::justPrint, glw::default_shader_error_handler());

	/*glgui::outlinedlabel lbtitle;
	glgui::button btconnect;
	lbtitle.pos = glm::ivec2(0, 50);
	lbtitle.charsize = glm::ivec2(25, 50);
	lbtitle.anch = glgui::anchor::TOPMID;
	lbtitle.align = glgui::anchor::TOPMID;
	lbtitle.color = glm::vec3(1, 1, 1);
	lbtitle.text = "Multiplayer-Game\nclient";
	lbtitle.init();
	btconnect.pos = glm::ivec2(0, 360);
	btconnect.size = glm::ivec2(400, 70);
	btconnect.charsize = glm::ivec2(25, 50);
	btconnect.anch = glgui::anchor::TOPMID;
	btconnect.align = glgui::anchor::TOPMID;
	btconnect.textalign = glgui::anchor::CENTER;
	btconnect.bgcolor = glm::vec3(1, 1, 1);
	btconnect.text = "connect";
	btconnect.init();*/
	lbtitle.pos = glm::ivec2(0, 50);
	lbtitle.anch = glgui::anchor::TOPMID;
	lbtitle.align = glgui::anchor::TOPMID;
	lbtitle.color = glm::vec3(1, 1, 1);
	lbtitle.outline = true;
	lbtitle.charsize = glm::ivec2(30, 60);
	lbtitle.setText("Multiplayer-game\nclient");
	gui.controls.push_back(&lbtitle);
	gui.pos = glm::ivec2(0, 0);
	gui.size = glm::ivec2(ww, wh);
	gui.anch = glgui::anchor::TOPLEFT;
	gui.align = glgui::anchor::TOPLEFT;
	gui.focused = false;
	gui.init();
}
void app::update() {
	double mx, my;
	glfwGetCursorPos(w, &mx, &my);
	glm::vec2 mouse(static_cast<float>(mx), static_cast<float>(my));
	if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		camorient += glm::vec2(mouse.y - prevm.y, prevm.x - mouse.x) * cfg.sensitivity;
		if (camorient.x >  1.57f) camorient.x = 1.57f;
		if (camorient.x < -1.57f) camorient.x = -1.57f;
		if (camorient.y >  glm::pi<float>()) camorient.y -= glm::pi<float>() * 2;
		if (camorient.y < -glm::pi<float>()) camorient.y += glm::pi<float>() * 2;
		prevm = mouse;
	}
	gui.update(dt);
}
void app::render() {
	renderGame();
	// draw posteffects
	glw::fbo::screen.bind();
	postsh.use();
	postsh.uniform1f("exposure", cfg.exposure);
	posttex.bind(GL_TEXTURE0);
	(cfg.bloomPasses > 0 ? tmp2tex : posttexover).bind(GL_TEXTURE1);
	quad.drawElements(6);
	if (stg == gamestage::MENU) {
		gui.render(glm::ortho<float>(0, ww, wh, 0));
	}
}
void app::renderGame() {
	// calculate camera view
	float camx = sinf(camorient.y) * cosf(camorient.x);
	float camy = sinf(camorient.x);
	float camz = cosf(camorient.y) * cosf(camorient.x);
	glm::vec3 cam = glm::normalize(glm::vec3(camx, camy, camz)) * CAM_DIST;
	glm::mat4 viewm = glm::lookAt(cam, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 vp = proj * viewm;
	glm::mat4 vr(1.f);
	vr = glm::rotate(vr, camorient.x, glm::vec3(1, 0, 0));
	vr = glm::rotate(vr, -camorient.y, glm::vec3(0, 1, 0));
	// target model matrix
	glm::mat4 trgmodel = glm::translate(glm::mat4(1.f),
		glm::vec3(
			trg.pos.x * 0.125f - 0.9375f,
			0.005f,
			trg.pos.y * 0.125f - 0.9375f));
	// shadow maps
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glm::mat4 sproj = glm::lookAt(sunpos / SUN_DIST * 1.2f, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	sproj = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, 0.1f, 2.5f) * sproj;
	glm::mat4 lproj = glm::lookAt(lamppos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	lproj = glm::perspective(2.f, 1.f, 0.1f, 2.5f) * lproj;
	if (cfg.shadows) {
		sunfbo.bind();
		glViewport(0, 0, cfg.shadowSize, cfg.shadowSize);
		glClear(GL_DEPTH_BUFFER_BIT);
		renderScene(sproj, lightsh);
		lightsh.use();
		lightsh.uniformM4f("proj", sproj * trgmodel);
		lightsh.uniformM4f("model", trgmodel);
		renderTarget();
		if (lamp) {
			lampfbo.bind();
			glClear(GL_DEPTH_BUFFER_BIT);
			renderScene(lproj, lightsh);
			lightsh.use();
			lightsh.uniformM4f("proj", lproj * trgmodel);
			lightsh.uniformM4f("model", trgmodel);
			renderTarget();
		}
	}
	// scene
	if (cfg.antialias) {
		glEnable(GL_MULTISAMPLE);
		postfboms.bind();
	} else {
		postfbo.bind();
	}
	glViewport(0, 0, ww, wh);
	glDrawBuffers(1, attachments + 1);
	glClearColor(0, 0, 0, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawBuffers(1, attachments);
	glClearColor(skycol.x, skycol.y, skycol.z, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawBuffers(2, attachments);
	glDisable(GL_DEPTH_TEST);
	trgsh.use();
	glm::mat4 sunmodel(1.f);
	glm::vec3 rendsunpos = sunpos / SUN_DIST * 25.f;
	sunmodel = glm::translate(sunmodel, rendsunpos);
	trgsh.uniformM4f("proj", proj * vr * sunmodel);
	trgsh.uniformM4f("model", sunmodel);
	trgsh.uniform3f("col", sunstrength * 20, sunstrength * 20, sunstrength * 20);
	sun.vao.bind();
	sun.draw();
	glEnable(GL_DEPTH_TEST);
	sh3d.use();
	sh3d.uniform3f("suncol", sunstrength, sunstrength, sunstrength);
	sh3d.uniform3f("lampcol", 1, 1, .6f);
	sh3d.uniform3f("sunpos", sunpos);
	sh3d.uniform3f("lamppos", lamppos);
	sh3d.uniform1i("lampon", lamp);
	sh3d.uniform3f("campos", cam);
	sh3d.uniformM4f("sunproj", sproj);
	sh3d.uniformM4f("lampproj", lproj);
	sh3d.uniform1i("shadows", cfg.shadows);
	sundepth.bind(GL_TEXTURE2);
	lampdepth.bind(GL_TEXTURE3);
	renderScene(vp, sh3d);
	trgsh.use();
	trgsh.uniformM4f("proj", vp * trgmodel);
	trgsh.uniformM4f("model", trgmodel);
	trgsh.uniform3f("col", colors::toRGB[trg.color]);
	renderTarget();
	if (cfg.antialias) {
		postfbomscolor0.bind(GL_READ_FRAMEBUFFER);
		postfbocolor0.bind(GL_DRAW_FRAMEBUFFER);
		postfbomscolor0.blit(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
			GL_NEAREST, ww, wh, ww, wh);
		postfbomscolor1.bind(GL_READ_FRAMEBUFFER);
		postfbocolor1.bind(GL_DRAW_FRAMEBUFFER);
		postfbomscolor1.blit(GL_COLOR_BUFFER_BIT, GL_NEAREST, ww, wh, ww, wh);
	}
	// blur of bloom
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	bool horizontal = 1;
	quad.bind();
	blursh.use();
	for (size_t i = 0; i < cfg.bloomPasses * 2; ++i, horizontal = !horizontal) {
		tmpfbo.bind();
		if (i == 0)
			posttexover.bind(GL_TEXTURE0);
		else
			tmp2tex.bind(GL_TEXTURE0);
		blursh.uniform1i("horizontal", horizontal);
		quad.drawElements(6);
		tmpfbo.swap(tmp2fbo);
		tmptex.swap(tmp2tex);
	}
}
void app::tick() {
	update();
	render();
	glw::checkError("tick end check", glw::justPrint);
}
void app::renderScene(const glm::mat4 &vp, glw::shader &sh) {
	sh.use();
	sh.uniformM4f("proj", vp * glm::mat4(1.f));
	sh.uniformM4f("model", glm::mat4(1.f));
	sh.uniform3f("col", 1, 1, 1);
	boardtex[0].bind(GL_TEXTURE0);
	boardtex[1].bind(GL_TEXTURE1);
	boardmesh.vao.bind();
	boardmesh.draw();

	for (size_t x = 0; x < 16; ++x) {
		for (size_t y = 0; y < 16; ++y) {
			for (size_t side = 0; side < 4; ++side) {
				if (brd.walls[x][y][side]) {
					glm::mat4 model = glm::mat4(1.f);
					model = glm::translate(model, glm::vec3(x * 0.125f - 0.9375f, 0, y * 0.125f - 0.9375f));
					model = glm::rotate(model, glm::pi<float>() * .5f * (side + 2), glm::vec3(0, 1, 0));
					sh.uniformM4f("proj", vp * model);
					sh.uniformM4f("model", model);
					sh.uniform3f("col", 1, 1, 1);
					walltex[0].bind(GL_TEXTURE0);
					walltex[1].bind(GL_TEXTURE1);
					wall.vao.bind();
					wall.draw();
				}
			}
		}
	}
	for (size_t i = 0; i < 5; ++i) {
		glm::mat4 model = glm::translate(glm::mat4(1.f),
			glm::vec3(
				bots[i].pos.x * 0.125f - 0.9375f,
				0.001f,
				bots[i].pos.y * 0.125f - 0.9375f));
		sh.uniformM4f("proj", vp * model);
		sh.uniformM4f("model", model);
		sh.uniform3f("col", colors::toRGB[bots[i].color]);
		whitetex.bind(GL_TEXTURE0);
		blacktex.bind(GL_TEXTURE1);
		robot.vao.bind();
		robot.draw();
	}
}
void app::renderTarget() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	robot.vao.bind();
	robot.draw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
