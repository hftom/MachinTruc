float PREFIX(hash)(float n) { return fract(pow(sin(n), 1.0) * 1e6); }

float PREFIX(snoise)(vec2 p) {
	p *= 2.5;
	const vec2 d = vec2(1.0 ,30.0);
	vec2 f = fract(p), p0 = floor(p) * d, p1 = p0 + d;
	f.xy = f.xy * f.xy * (3.0 - 2.0 * f.xy);
	float t = mix(mix(PREFIX(hash)(p0.x + p0.y), PREFIX(hash)(p1.x + p0.y), f.x),
		       mix(PREFIX(hash)(p0.x + p1.y), PREFIX(hash)(p1.x + p1.y), f.x), f.y);
	return t * 2.0 - 1.0;
}

float PREFIX(fbm)( vec2 p) {
	float f = 0.0;
	f += 0.5000 * PREFIX(snoise)(p); p *= 2.22;
	f += 0.2500 * PREFIX(snoise)(p); p *= 2.03;
	f += 0.1250 * PREFIX(snoise)(p); p *= 2.01;
	f += 0.0625 * PREFIX(snoise)(p); p *= 2.04;
	return (f / 0.9375);
}

vec3 PREFIX(sun)( vec2 pos ) {
	vec2 p = vec2(0.75, 0.9);
	return vec3(2.5, 0.5, 0.0) / (distance( p, pos ) * 25.0);
}

vec4 FUNCNAME(vec2 tc) {
	vec2 p = tc * 2.0 - 1.0;
	p.x /= PREFIX(iheight) / PREFIX(iwidth);
	vec3 c1 = mix(vec3(-0.5), vec3(0.0, 0.2, 1.0), tc.y);
	float c2 = PREFIX(fbm)(p - PREFIX(time) / 10.0) + PREFIX(fbm)(p - PREFIX(time) / 20.0) + PREFIX(fbm)(p - PREFIX(time) / 60.0) + 11.0;
	float v = c2 * 0.06; //(((c2 * 0.1) - 0.35) * 0.45) + 0.3;
	vec4 col = vec4( mix( c1, vec3(v), v ), 1.0 );
	return vec4( col.rgb + PREFIX(sun)( tc ), 1.0);
}
