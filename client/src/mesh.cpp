#include "mesh.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>

mesh::mesh() { }
void mesh::load(const std::string &file) {
	std::vector<glm::vec3> v;
	std::vector<glm::vec2> vt;
	std::vector<glm::vec3> vn;
	std::vector<float> verts;
	std::vector<unsigned int> inds;

	std::ifstream f(file);
	std::string line;
	while (std::getline(f, line)) {
		std::istringstream ss(line);
		ss >> std::ws;
		std::string t;
		ss >> t;
		if (t == "v") {
			float x, y, z;
			ss >> x >> y >> z;
			v.emplace_back(x, y, z);
		} else if (t == "vt") {
			float x, y;
			ss >> x >> y;
			vt.emplace_back(x, y);
		} else if (t == "vn") {
			float x, y, z;
			ss >> x >> y >> z;
			vn.emplace_back(x, y, z);
		} else if (t == "f") {
			const unsigned int first = verts.size() / 8;
			unsigned int c = 0;
			while (ss >> t) {
				size_t i = 0;
				unsigned int vs[3];
				size_t last = 0;
				size_t next = 0;
				while ((next = t.find('/', last)) != std::string::npos) {
					vs[i++] = std::stoi(t.substr(last, next-last));
					last = next + 1;
				}
				vs[i++] = std::stoi(t.substr(last));
				while (i < 3) {
					vs[i++] = vs[0];
				}
				verts.emplace_back(v[vs[0]-1].x);
				verts.emplace_back(v[vs[0]-1].y);
				verts.emplace_back(v[vs[0]-1].z);
				verts.emplace_back(vn[vs[2]-1].x);
				verts.emplace_back(vn[vs[2]-1].y);
				verts.emplace_back(vn[vs[2]-1].z);
				verts.emplace_back(vt[vs[1]-1].x);
				verts.emplace_back(vt[vs[1]-1].y);
				++c;
			}
			for (unsigned int i = 2; i < c; ++i) {
				inds.emplace_back(first);
				inds.emplace_back(first + i - 1);
				inds.emplace_back(first + i);
			}
		}
	}
	glw::initVaoVboEbo(vao, vbo, ebo, verts.data(), sizeof(float)*verts.size(),
		inds.data(), sizeof(unsigned int)*inds.size(),
		sizeof(float)*8, {glw::vap(3), glw::vap(3, sizeof(float)*3), glw::vap(2, sizeof(float)*6)});
	vertexcount = inds.size();
}
void mesh::draw() {
	vao.drawElements(vertexcount);
}
