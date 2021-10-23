#version 400 core
in vec2 tpos;
out vec4 outcol;

uniform vec3 lightcol;
uniform vec3 col;
uniform sampler2D tex;

void main() {
	vec4 sampled = texture(tex, tpos);
	if (sampled.a < 0.5)
		discard;
	outcol = vec4(sampled.xyz*lightcol*col, 1);
}
