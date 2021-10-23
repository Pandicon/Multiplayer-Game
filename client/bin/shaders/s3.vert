#version 400 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texpos;
out vec3 fpos;
out vec3 norm;
out vec2 tpos;

uniform mat4 proj;
uniform mat4 model;

void main() {
	gl_Position = proj * vec4(pos, 1);
	fpos = (model * vec4(pos, 1)).xyz;
	norm = normal;//mat3(transpose(inverse(model))) * normal;
	tpos = texpos;
}
