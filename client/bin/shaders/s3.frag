#version 400 core
in vec3 fpos;
in vec3 norm;
in vec2 tpos;
in vec4 sunrpos;
in vec4 lamprpos;
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
uniform sampler2D sundepth;
uniform sampler2D lampdepth;

const float specfactor = 0.5;
const float bias = 0.005;

float getShadow(vec3 flpos, sampler2D smap) {
	flpos = flpos * 0.5 + 0.5;
	float closest = texture(smap, flpos.xy).r; 
	return flpos.z - bias > closest ? 0.0 : 1.0;
}

vec2 calcLight(vec3 lpos, float ambient, vec4 flpos, sampler2D smap) {
	vec3 lightdir = normalize(lpos - fpos);
	vec3 viewDir = normalize(campos - fpos);
	vec3 halfdir = normalize(lightdir + viewDir);
	float diff = max(dot(norm, lightdir), 0.0);
	float spec = pow(max(dot(norm, halfdir), 0.0), 8.0);
	float light = getShadow(flpos.xyz / flpos.w, smap);
	return vec2(ambient + diff * light, spec * specfactor * light);
}

void main() {
	vec4 sampled = texture(tex, tpos);
	if (sampled.a < 0.5)
		discard;
	vec4 sampledspec = texture(texspec, tpos);
	vec2 light = calcLight(sunpos, 0.3, sunrpos, sundepth);
	vec3 diff = sampled.xyz * light.x * suncol;
	vec3 spec = sampledspec.xyz * light.y * suncol;
	if (lampon) {
		light = calcLight(lamppos, 0.1, lamprpos, lampdepth);
		diff = sampled.xyz * light.x * lampcol;
		spec = sampledspec.xyz * light.y * lampcol;
	}
	outcol = vec4(col * (diff + spec), 1);
}
