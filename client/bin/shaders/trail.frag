#version 400 core
in vec3 color;

layout (location = 0) out vec4 outcol;
layout (location = 1) out vec4 outcolover;

void main() {
	outcol = vec4(color, 1);
	outcolover = vec4(color, 1);
}
