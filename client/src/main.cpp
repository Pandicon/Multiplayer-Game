#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "app.hpp"

app *a;

void resize_main(GLFWwindow *w, int x, int y) {
	(void)w;
	a->resize(x, y);
}
void click_main(GLFWwindow *w, int btn, int act, int mod) {
	(void)w;
	a->click(btn, act, mod);
}
void scroll_main(GLFWwindow *w, double x, double y) {
	(void)w;
	a->scroll(x, y);
}
void key_main(GLFWwindow* w, int key, int scancode, int action, int mods) {
	(void)w;
	(void)scancode;
	a->key(key, action, mods);
}
void character_main(GLFWwindow* w, unsigned int codepoint) {
	(void)w;
	a->write(codepoint);
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
	glfwSetMouseButtonCallback(a->w, click_main);
	glfwSetScrollCallback(a->w, scroll_main);
	glfwSetKeyCallback(a->w, key_main);
	glfwSetCharCallback(a->w, character_main);
	a->mainloop();
	delete a;
	glfwTerminate();
	return 0;
}

#define GLW_HPP_DECLS
#include "glw.hpp"
#define GLGUI_IMPL
#include "glgui.hpp"
