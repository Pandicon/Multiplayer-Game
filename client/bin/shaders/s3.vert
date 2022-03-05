#version 400 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texpos;
out vec3 fpos;
out vec3 norm;
out vec2 tpos;
out vec4 sunrpos;
out vec4 lamprpos;

uniform mat4 proj;
uniform mat4 model;
uniform mat4 sunproj;
uniform mat4 lampproj;

void main() {
	gl_Position = proj * vec4(pos, 1);
	fpos = (model * vec4(pos, 1)).xyz;
	norm = normalize(mat3(model) * normal);//mat3(transpose(inverse(model))) * normal;
	tpos = texpos;
	sunrpos = sunproj * model * vec4(pos, 1);
	lamprpos = lampproj * model * vec4(pos, 1);
}
