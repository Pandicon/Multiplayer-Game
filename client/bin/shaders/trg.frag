#version 400 core
layout (location = 0) out vec4 outcol;
layout (location = 1) out vec4 outcolover;

uniform vec3 col;

void main() {
	outcol = vec4(col, 1);
	outcolover = vec4(col, 1);
}
