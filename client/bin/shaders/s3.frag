#version 400 core
in vec3 fpos;
in vec3 norm;
in vec2 tpos;
out vec4 outcol;

uniform vec3 suncol;
uniform vec3 lampcol;
uniform vec3 sunpos;
uniform vec3 lamppos;
uniform bool lampon;
uniform vec3 col;
uniform vec3 campos;
uniform sampler2D tex;
uniform sampler2D texspec;

const float gamma = 2.2;
const float specfactor = 0.5;

vec2 calcLight(vec3 lpos, float ambient) {
	vec3 lightdir = normalize(lpos - fpos);
	vec3 viewDir = normalize(campos - fpos);
	vec3 halfdir = normalize(lightdir + viewDir);
	float diff = max(dot(norm, lightdir), 0.0);
	float spec = pow(max(dot(norm, halfdir), 0.0), 8.0);
	return vec2(ambient + diff, spec * specfactor);
}

void main() {
	vec4 sampled = texture(tex, tpos);
	if (sampled.a < 0.5)
		discard;
	vec4 sampledspec = texture(texspec, tpos);
	vec2 light = calcLight(sunpos, 0.3);
	vec3 diff = sampled.xyz * light.x * suncol;
	vec3 spec = sampledspec.xyz * light.y * suncol;
	if (lampon) {
		light = calcLight(lamppos, 0.1);
		diff = sampled.xyz * light.x * lampcol;
		spec = sampledspec.xyz * light.y * lampcol;
	}
	outcol = vec4(col * (diff + spec), 1);
    outcol.rgb = pow(outcol.rgb, vec3(1.0/gamma));
}
