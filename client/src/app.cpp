#include "app.hpp"

#include <iostream>

app::app(int ww, int wh, const char *title) {
	w = glfwCreateWindow(ww, wh, title, NULL, NULL);
	glfwMakeContextCurrent(w);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "OpenGL load failed. Does your graphics card support OpenGL 4.0 core?" << std::endl;
	}
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
	}
}
void app::resize(int ww, int wh) {
	glViewport(0, 0, ww, wh);
}

void app::tick() {
	glClearColor(.2f, .7f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);
}
