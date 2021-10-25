#version 400 core
in vec2 tpos;
out vec4 outcol;

uniform sampler2D tex;
uniform float exposure;

const float gamma = 2.2;

void main() {
	outcol = vec4(texture(tex, tpos).rgb, 1);
	outcol.rgb = vec3(1.0) - exp(-outcol.rgb * exposure);
	outcol.rgb = pow(outcol.rgb, vec3(1.0/gamma));
}
