#version 400 core
in vec2 tpos;
out vec4 outcol;

uniform sampler2D tex;

void main() {
	outcol = vec4(vec3(texture(tex, tpos).r), 1);
}
