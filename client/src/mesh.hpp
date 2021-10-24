#ifndef __MESH_HPP__
#define __MESH_HPP__

#include <string>
#include "glw.hpp"

class mesh {
public:
	glw::vao vao;
	unsigned int vertexcount;

	mesh();
	void load(const std::string &file);
	void draw();
private:
	glw::vbo vbo;
	glw::ebo ebo;
};

#endif
