#version 400 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 col; // instead normal (keep compatible with light shader)
layout (location = 2) in vec2 pad;

out vec3 color;

uniform mat4 proj;

void main() {
	gl_Position = proj * vec4(pos, 1);
	color = col;
}
