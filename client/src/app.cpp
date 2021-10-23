#include "app.hpp"

#include <iostream>

app::app(int ww, int wh, const char *title) {
	w = glfwCreateWindow(ww, wh, title, NULL, NULL);
	glfwMakeContextCurrent(w);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "OpenGL load failed. Does your graphics card support OpenGL 4.0 core?" << std::endl;
	}
	resize(ww, wh);

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
	glw::compileShaderFromFile(postsh, "./shaders/post", glw::default_shader_error_handler());
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
void app::resize(int ww, int wh) {
	glViewport(0, 0, ww, wh);
}

void app::tick() {
	glClearColor(.2f, .7f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
	postsh.use();
	postsh.uniformM4f("proj", glm::mat4(1.f));
	postsh.uniform3f("col", 1, 1, 1);
	quad.bind();
	quad.drawElements(6);
	//quad.drawArrays(4);
	glw::checkError("tick end check", glw::justPrint);
}
