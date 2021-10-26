#version 400 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 texpos;
out vec2 tpos;

void main() {
	gl_Position = vec4(pos, 0, 1);
	tpos = texpos;
}
