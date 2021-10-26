#version 400 core
in vec2 tpos;
out vec4 outcol;

uniform sampler2D tex;
  
uniform bool horizontal;
uniform float w[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
	vec2 tex_offset = 1.0 / textureSize(tex, 0);
	vec3 result = texture(tex, tpos).rgb * w[0];
	if(horizontal) {
		for(int i = 1; i < 5; ++i) {
			result += texture(tex, tpos + vec2(tex_offset.x * i, 0.0)).rgb * w[i];
			result += texture(tex, tpos - vec2(tex_offset.x * i, 0.0)).rgb * w[i];
		}
	} else {
		for(int i = 1; i < 5; ++i) {
			result += texture(tex, tpos + vec2(0.0, tex_offset.y * i)).rgb * w[i];
			result += texture(tex, tpos - vec2(0.0, tex_offset.y * i)).rgb * w[i];
		}
	}
	outcol = vec4(result, 1.0);
}