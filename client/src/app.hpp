#ifndef __APP_HPP__
#define __APP_HPP__

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class app {
public:
	GLFWwindow *w;

	app(int ww, int wh, const char *title);
	void mainloop();
	void resize(int ww, int wh);
private:
	float dt;

	void tick();
};

#endif
