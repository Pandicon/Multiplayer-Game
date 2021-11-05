#include "app.hpp"

#include <math.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <glm/gtc/matrix_transform.hpp>

constexpr float CAM_DIST = 1.6f;
constexpr float SUN_DIST = 1000;
constexpr float SENSITIVITY = 0.01f;
constexpr int SHADOW_RESOLUTION = 1024;

app::app(int ww, int wh, const char *title) : camorient(1, 0) {
	w = glfwCreateWindow(ww, wh, title, NULL, NULL);
	glfwMakeContextCurrent(w);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "OpenGL load failed. Does your graphics card support OpenGL 4.0 core?" << std::endl;
	}

	auto now = std::chrono::system_clock::now();
	time_t tnow = std::chrono::system_clock::to_time_t(now);
    tm *date = std::localtime(&tnow);
    date->tm_hour = 0;
    date->tm_min = 0;
    date->tm_sec = 0;
    auto midnight = std::chrono::system_clock::from_time_t(std::mktime(date));
	auto time = std::chrono::duration_cast<std::chrono::seconds>(now - midnight).count();
	float dayprogress = static_cast<float>(time) / 86400.f;
	sunpos = glm::vec3(sinf(dayprogress*2.f*glm::pi<float>()), -cosf(dayprogress*2.f*glm::pi<float>()), 0.f);
	skycol = glm::vec3(.2f, .7f, 1.f) * (sunpos.y < 0 ? 0 : sunpos.y);
	lamp = sunpos.y < 0.1f;
	lamppos = glm::vec3(0.f, 1.0f, 0.1f);
	sunpos *= SUN_DIST;

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
	float boardverts[] = {
	//  position   normal   texture-pos
		-1, 0, -1, 0,  1, 0, 0, 0,
		-1, 0,  1, 0,  1, 0, 0, 1,
		 1, 0,  1, 0,  1, 0, 1, 1,
		 1, 0, -1, 0,  1, 0, 1, 0,

		-1, 0, -1, 0, -1, 0, 0, 0,
		 1, 0, -1, 0, -1, 0, 1, 0,
		 1, 0,  1, 0, -1, 0, 1, 1,
		-1, 0,  1, 0, -1, 0, 0, 1,
	};
	unsigned int boardindices[] = {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7
	};
	glw::initVaoVboEbo(board, boardvbo, boardebo, boardverts, sizeof(boardverts),
		boardindices, sizeof(boardindices), sizeof(float)*8,
		{glw::vap(3), glw::vap(3, sizeof(float)*3), glw::vap(2, sizeof(float)*6)});
	const float wsx = -.0625;
	const float wex = -.046875f;
	const float wsy =  .0f;
	const float wey =  .03f;
	const float wsz = -.06249;
	const float wez =  .06249f;
	float wallverts[] = {
	//  position       normal    texture-pos
		wsx, wsy, wsz, -1, 0, 0, 0, .5f,
		wsx, wsy, wez, -1, 0, 0, 1, .5f,
		wsx, wey, wez, -1, 0, 0, 1, 0,
		wsx, wey, wsz, -1, 0, 0, 0, 0,

		wex, wsy, wsz,  1, 0, 0, 0, .5f,
		wex, wey, wsz,  1, 0, 0, 0, 0,
		wex, wey, wez,  1, 0, 0, 1, 0,
		wex, wsy, wez,  1, 0, 0, 1, .5f,

		wsx, wsy, wsz, 0, 0, -1, 0, 0,
		wsx, wey, wsz, 0, 0, -1, 1, 0,
		wex, wey, wsz, 0, 0, -1, 1, .5f,
		wex, wsy, wsz, 0, 0, -1, 0, .5f,

		wsx, wsy, wez, 0, 0, 1, 0, 0,
		wex, wsy, wez, 0, 0, 1, 0, .5f,
		wex, wey, wez, 0, 0, 1, 1, .5f,
		wsx, wey, wez, 0, 0, 1, 1, 0,

		wsx, wey, wsz, 0,  1, 0, 0, .5f,
		wsx, wey, wez, 0,  1, 0, 1, .5f,
		wex, wey, wez, 0,  1, 0, 1, 1,
		wex, wey, wsz, 0,  1, 0, 0, 1,
	};
	unsigned int wallindices[] = {
		0,  1,  2,  0,  2,  3,
		4,  5,  6,  4,  6,  7,
		8,  9,  10, 8,  10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19
	};
	glw::initVaoVboEbo(wall, wallvbo, wallebo, wallverts, sizeof(wallverts),
		wallindices, sizeof(wallindices), sizeof(float)*8,
		{glw::vap(3), glw::vap(3, sizeof(float)*3), glw::vap(2, sizeof(float)*6)});
	robot.load("./models/robot.obj");

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

	sunfbo.gen();
	sundepth.gen();
	sundepth.bind();
	sundepth.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	sundepth.size = glm::ivec2(SHADOW_RESOLUTION, SHADOW_RESOLUTION);
	sundepth.upload(NULL, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
	sunfbo.bind();
	sunfbo.attach(sundepth, GL_DEPTH_ATTACHMENT);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	lampfbo.gen();
	lampdepth.gen();
	lampdepth.bind();
	lampdepth.setWrapFilter({GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE}, GL_NEAREST, GL_NEAREST);
	lampdepth.size = glm::ivec2(SHADOW_RESOLUTION, SHADOW_RESOLUTION);
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
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	glw::fbo::screen.bind();
	glw::checkError("init check", glw::justPrint);
	resize(ww, wh);
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
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
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
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
}
void app::resize(int ww, int wh) {
	this->ww = ww;
	this->wh = wh;
	proj = glm::perspective(2.f, static_cast<float>(ww) / wh, .1f, 100.f);
	posttex.bind();
	posttex.size = glm::ivec2(ww, wh);
	posttex.upload(NULL, GL_RGBA, GL_RGBA16F, GL_FLOAT);
	posttexover.bind();
	posttexover.size = glm::ivec2(ww, wh);
	posttexover.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	postdepth.bind();
	postdepth.size = glm::ivec2(ww, wh);
	postdepth.upload(NULL, GL_DEPTH_STENCIL, GL_DEPTH24_STENCIL8, GL_UNSIGNED_INT_24_8);
	tmptex.bind();
	tmptex.size = glm::ivec2(ww, wh);
	tmptex.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
	tmp2tex.bind();
	tmp2tex.size = glm::ivec2(ww, wh);
	tmp2tex.upload(NULL, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
}

void app::tick() {
	double mx, my;
	glfwGetCursorPos(w, &mx, &my);
	glm::vec2 mouse(static_cast<float>(mx), static_cast<float>(my));
	if (glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		camorient += glm::vec2(mouse.y - prevm.y, prevm.x - mouse.x) * SENSITIVITY;
		if (camorient.x >  1.57f) camorient.x = 1.57f;
		if (camorient.x < -1.57f) camorient.x = -1.57f;
		if (camorient.y >  glm::pi<float>()) camorient.y -= glm::pi<float>() * 2;
		if (camorient.y < -glm::pi<float>()) camorient.y += glm::pi<float>() * 2;
	}
	prevm = mouse;
	float camx = sinf(camorient.y) * cosf(camorient.x);
	float camy = sinf(camorient.x);
	float camz = cosf(camorient.y) * cosf(camorient.x);
	glm::vec3 cam = glm::normalize(glm::vec3(camx, camy, camz)) * CAM_DIST;
	glm::mat4 viewm = glm::lookAt(cam, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 vp = proj * viewm;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	sunfbo.bind();
	glViewport(0, 0, SHADOW_RESOLUTION, SHADOW_RESOLUTION);
	glClear(GL_DEPTH_BUFFER_BIT);
	glm::mat4 sproj = glm::lookAt(sunpos / SUN_DIST * 1.2f, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	sproj = glm::ortho(-1.2f, 1.2f, -1.2f, 1.2f, 0.1f, 2.5f) * sproj;
	render(sproj, lightsh);
	glm::mat4 lproj = glm::lookAt(lamppos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	lproj = glm::perspective(2.f, 1.f, 0.1f, 2.5f) * lproj;
	if (lamp) {
		lampfbo.bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		render(lproj, lightsh);
	}

	postfbo.bind();
	glViewport(0, 0, ww, wh);
	
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(1, attachments + 1);
	glClearColor(0, 0, 0, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawBuffers(1, attachments);
	glClearColor(skycol.x, skycol.y, skycol.z, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawBuffers(2, attachments);
	sh3d.use();
	sh3d.uniform3f("suncol", 1, 1, 1);
	sh3d.uniform3f("lampcol", 1, 1, .6f);
	sh3d.uniform3f("sunpos", sunpos);
	sh3d.uniform3f("lamppos", lamppos);
	sh3d.uniform1i("lampon", lamp);
	sh3d.uniform3f("campos", cam);
	sh3d.uniformM4f("sunproj", sproj);
	sh3d.uniformM4f("lampproj", lproj);
	sundepth.bind(GL_TEXTURE2);
	lampdepth.bind(GL_TEXTURE3);
	render(vp, sh3d);
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	bool horizontal = 1;
	quad.bind();
	blursh.use();
	for (size_t i = 0; i < 8; ++i, horizontal = !horizontal) {
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
	glw::fbo::screen.bind();
	postsh.use();
	postsh.uniform1f("exposure", 1.5f);
	posttex.bind(GL_TEXTURE0);
	tmp2tex.bind(GL_TEXTURE1);
	quad.drawElements(6);
	glw::checkError("tick end check", glw::justPrint);
}
void app::render(const glm::mat4 &vp, glw::shader &sh) {
	sh.use();
	sh.uniformM4f("proj", vp * glm::mat4(1.f));
	sh.uniformM4f("model", glm::mat4(1.f));
	sh.uniform3f("col", 1, 1, 1);
	boardtex[0].bind(GL_TEXTURE0);
	boardtex[1].bind(GL_TEXTURE1);
	board.bind();
	board.drawElements(12);

	for (size_t x = 0; x < 16; ++x) {
		for (size_t y = 0; y < 16; ++y) {
			for (size_t side = 0; side < 4; ++side) {
				glm::mat4 model = glm::rotate(glm::mat4(1.f), glm::pi<float>() * .5f * side, glm::vec3(0, 1, 0));
				model = glm::translate(model, glm::vec3(x * 0.125f - 0.9375f, 0, y * 0.125f - 0.9375f));
				sh.uniformM4f("proj", vp * model);
				sh.uniformM4f("model", model);
				sh.uniform3f("col", 1, 1, 1);
				walltex[0].bind(GL_TEXTURE0);
				walltex[1].bind(GL_TEXTURE1);
				wall.bind();
				wall.drawElements(30);
			}
		}
	}
	glm::mat4 model =
		glm::translate(glm::mat4(1.f), glm::vec3(0 * 0.125f - 0.9375f, 0.001f, 0 * 0.125f - 0.9375f));
	sh.uniformM4f("proj", vp * model);
	sh.uniformM4f("model", model);
	sh.uniform3f("col", 10, 10, 0);
	whitetex.bind(GL_TEXTURE0);
	blacktex.bind(GL_TEXTURE1);
	robot.vao.bind();
	robot.draw();
	model = glm::translate(glm::mat4(1.f), glm::vec3(1 * 0.125f - 0.9375f, 0.001f, 0 * 0.125f - 0.9375f));
	sh.uniformM4f("proj", vp * model);
	sh.uniformM4f("model", model);
	sh.uniform3f("col", 1, 0, 0);
	whitetex.bind(GL_TEXTURE0);
	blacktex.bind(GL_TEXTURE1);
	robot.vao.bind();
	robot.draw();
	model = glm::translate(glm::mat4(1.f), glm::vec3(2 * 0.125f - 0.9375f, 0.001f, 0 * 0.125f - 0.9375f));
	sh.uniformM4f("proj", vp * model);
	sh.uniformM4f("model", model);
	sh.uniform3f("col", 0, 0.7f, 0);
	whitetex.bind(GL_TEXTURE0);
	blacktex.bind(GL_TEXTURE1);
	robot.vao.bind();
	robot.draw();
	model = glm::translate(glm::mat4(1.f), glm::vec3(0 * 0.125f - 0.9375f, 0.001f, 3 * 0.125f - 0.9375f));
	sh.uniformM4f("proj", vp * model);
	sh.uniformM4f("model", model);
	sh.uniform3f("col", 0, 0, 1);
	whitetex.bind(GL_TEXTURE0);
	blacktex.bind(GL_TEXTURE1);
	robot.vao.bind();
	robot.draw();
	model = glm::translate(glm::mat4(1.f), glm::vec3(0 * 0.125f - 0.9375f, 0.001f, 4 * 0.125f - 0.9375f));
	sh.uniformM4f("proj", vp * model);
	sh.uniformM4f("model", model);
	sh.uniform3f("col", .2f, .2f, .2f);
	whitetex.bind(GL_TEXTURE0);
	blacktex.bind(GL_TEXTURE1);
	robot.vao.bind();
	robot.draw();
}
