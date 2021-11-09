#version 400 core
in vec2 tpos;

out vec4 outcol;

uniform bool usetex;
uniform vec3 col;
uniform sampler2D tex;

void main() {
	if (usetex) {
		outcol = texture(tex, tpos);
		if (outcol.a < 0.5)
			discard;
		outcol.xyz *= col;
	} else {
		outcol = vec4(col, 1);
	}
}
