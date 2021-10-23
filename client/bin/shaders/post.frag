#version 400 core
in vec2 tpos;
out vec4 outcol;

uniform vec3 col;

void main() {
	outcol = vec4(col, 1);
}
