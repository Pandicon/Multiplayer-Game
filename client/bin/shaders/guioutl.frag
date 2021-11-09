#version 400 core
in vec2 tpos;
out vec4 outcol;

uniform vec3 col;
uniform sampler2D tex;

const float offset = 0.005;

void main() {
	vec4 sampled = texture(tex, tpos);
	if (sampled.a > 0.5) {
		outcol = vec4(sampled.xyz * col, 1);
	} else {
		vec2 off[4] = vec2[4](
			vec2( offset, 0),
			vec2(-offset, 0),
			vec2(0,  offset),
			vec2(0, -offset)
		);
		vec2 boundmin = floor(tpos * 16) * 0.0625;
		vec2 boundmax = boundmin + vec2(.0625, .0625);
		for (int i = 0; i < 4; ++i) {
			vec2 searchpos = tpos + off[i];
			if (searchpos.x < boundmin.x || searchpos.y < boundmin.y ||
				searchpos.x > boundmax.x || searchpos.y > boundmax.y)
				continue;
			vec4 sampledaround = texture(tex, searchpos);
			if (sampledaround.a > 0.5) {
				if (dot(sampledaround.rgb * col, vec3(0.299, 0.587, 0.114)) < 0.5)
					outcol = vec4(1, 1, 1, 1);
				else
					outcol = vec4(0, 0, 0, 1);
				return;
			}
		}
		discard;
	}
}
