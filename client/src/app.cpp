#include "app.hpp"

#include <math.h>
#include <chrono>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

constexpr float CAM_DIST = 1.5f;
constexpr float SENSITIVITY = 0.01f;

app::app(int ww, int wh, const char *title) : camorient(1, 0) {
	w = glfwCreateWindow(ww, wh, title, NULL, NULL);
	glfwMakeContextCurrent(w);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "OpenGL load failed. Does your graphics card support OpenGL 4.0 core?" << std::endl;
	}
	resize(ww, wh);

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
	lamppos = glm::vec3(0.f, 1.f, 0.f);
	sunpos *= 30;

	float quadverts[] = {
		-1, -1, 0, 0,
		-1,  1, 0, 1,
		 1,  1, 1, 1,
		 1, -1, 1, 0,
	};
	unsigned int quadindices[] = {
		0, 1, 2,
		0, 2, 3
	};
	glw::initVaoVboEbo(quad, quadvbo, quadebo, quadverts, sizeof(quadverts),
		quadindices, sizeof(quadindices), sizeof(float)*4, {glw::vap(2),glw::vap(2, sizeof(float)*2)});
	float boardverts[] = {
	//  position   normal   texture-pos
		-1, 0, -1, 0, 1, 0, 0, 0,
		-1, 0,  1, 0, 1, 0, 0, 1,
		 1, 0,  1, 0, 1, 0, 1, 1,
		 1, 0, -1, 0, 1, 0, 1, 0,
	};
	unsigned int boardindices[] = {
		0, 1, 2,
		0, 2, 3
	};
	glw::initVaoVboEbo(board, boardvbo, boardebo, boardverts, sizeof(boardverts),
		boardindices, sizeof(boardindices), sizeof(float)*8,
		{glw::vap(3), glw::vap(3, sizeof(float)*3), glw::vap(2, sizeof(float)*6)});
	const float wsx = -.0625;
	const float wex = -.046875f;
	const float wsy =  .0f;
	const float wey =  .03f;
	const float wsz = -.062499;
	const float wez =  .062499f;
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

		wsx, wsy, wez, 0, 0,  1, 0, 0,
		wex, wey, wez, 0, 0,  1, 1, 0,
		wex, wey, wez, 0, 0,  1, 1, .5f,
		wsx, wsy, wez, 0, 0,  1, 0, .5f,

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
	glw::compileShaderFromFile(postsh, "./shaders/post", glw::default_shader_error_handler());
	glw::compileShaderFromFile(sh3d, "./shaders/s3", glw::default_shader_error_handler());
	sh3d.use();
	sh3d.uniform1i("tex", 0);
	sh3d.uniform1i("texspec", 1);
	boardtex.gen();
	boardtex.bind();
	boardtex.setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_LINEAR, GL_LINEAR);
	boardtex.fromFile("./textures/board.png", glw::justPrint, "Could not find", GL_RGBA, GL_SRGB_ALPHA);
	boardspectex.gen();
	boardspectex.bind();
	boardspectex.setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_LINEAR, GL_LINEAR);
	boardspectex.fromFile("./textures/board_spec.png", glw::justPrint, "Could not find", GL_RGBA, GL_RGBA8);
	walltex.gen();
	walltex.bind();
	walltex.setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_LINEAR, GL_LINEAR);
	walltex.fromFile("./textures/wall.png", glw::justPrint, "Could not find", GL_RGBA, GL_SRGB_ALPHA);
	wallspectex.gen();
	wallspectex.bind();
	wallspectex.setWrapFilter({GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE}, GL_LINEAR, GL_LINEAR);
	wallspectex.fromFile("./textures/wall_spec.png", glw::justPrint, "Could not find", GL_RGBA, GL_RGBA8);
	glw::checkError("init check", glw::justPrint);
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
	}
}
void app::resize(int ww, int wh) {
	glViewport(0, 0, ww, wh);
	proj = glm::perspective(2.f, static_cast<float>(ww) / wh, .1f, 100.f);
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

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(skycol.x, skycol.y, skycol.z, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 viewm = glm::lookAt(cam, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 vp = proj * viewm;

	sh3d.use();
	sh3d.uniform3f("suncol", 1, 1, 1);
	sh3d.uniform3f("lampcol", 1, 1, .7f);
	sh3d.uniform3f("sunpos", sunpos);
	sh3d.uniform3f("lamppos", lamppos);
	sh3d.uniform1i("lampon", lamp);
	sh3d.uniform3f("campos", cam);

	sh3d.uniformM4f("proj", vp * glm::mat4(1.f));
	sh3d.uniformM4f("model", glm::mat4(1.f));
	sh3d.uniform3f("col", 1, 1, 1);
	boardtex.bind(GL_TEXTURE0);
	boardspectex.bind(GL_TEXTURE1);
	board.bind();
	board.drawElements(6);

	for (size_t x = 0; x < 16; ++x) {
		for (size_t y = 0; y < 16; ++y) {
			for (size_t side = 0; side < 4; ++side) {
				glm::mat4 model = glm::rotate(glm::mat4(1.f), glm::pi<float>() * .5f * side, glm::vec3(0, 1, 0));
				model = glm::translate(model, glm::vec3(x * 0.125f - 0.9375f, 0, y * 0.125f - 0.9375f));
				sh3d.uniformM4f("proj", vp * model);
				sh3d.uniformM4f("model", model);
				sh3d.uniform3f("col", 1, 1, 1);
				walltex.bind(GL_TEXTURE0);
				wallspectex.bind(GL_TEXTURE1);
				wall.bind();
				wall.drawElements(30);
			}
		}
	}
	glw::checkError("tick end check", glw::justPrint);
}
