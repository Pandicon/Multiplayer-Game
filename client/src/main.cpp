#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "app.hpp"

app *a;

void resize_main(GLFWwindow *w, int x, int y) {
	(void)w;
	a->resize(x, y);
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
	a = new app(800, 600, "Multiplayer Game - client");
	glfwSetFramebufferSizeCallback(a->w, resize_main);
	a->mainloop();
	delete a;
	glfwTerminate();
	return 0;
}

#define GLW_HPP_DECLS
#include "glw.hpp"
#define GLGUI_IMPL
#include "glgui.hpp"
