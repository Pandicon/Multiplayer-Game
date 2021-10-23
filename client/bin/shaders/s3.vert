#version 400 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texpos;
out vec2 tpos;

uniform mat4 proj;

void main() {
	gl_Position = proj * vec4(pos, 1);
	tpos = texpos;
}
