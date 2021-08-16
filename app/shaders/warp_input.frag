float PREFIX(hash)( float n ) {
    return fract(sin(n) * 9.0);
}

float PREFIX(noise)( in vec2 x ) {
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    float n = p.x + p.y * 7919.0;
    return mix(mix( PREFIX(hash)(n + 0.0), PREFIX(hash)(n + 1.0), f.x), mix( PREFIX(hash)(n + 7919.0), PREFIX(hash)(n + 7920.0),f.x),f.y);
}

float PREFIX(fbm)( vec2 p ) {
    return (0.25 * PREFIX(noise)( p * 3.0 )) / 0.38375;
}

vec4 FUNCNAME( vec2 tc ) {
	vec2 p = -1.0 + 2.0 * tc;
	p.x *= PREFIX(iwidth) / PREFIX(iheight);

	vec2 dx1 = vec2(1.0, 1.0);
	vec2 dy1 = vec2(1.2, 1.3);

	vec2 dx2 = vec2(1.6, 1.3);
	vec2 dy2 = vec2(1.2, 1.6);

	dx1 = dx1 * mat2(cos(PREFIX(time) / 5.33), -sin(PREFIX(time) / 2.66), -sin(PREFIX(time) / 2.33), cos(PREFIX(time) / 3.33));

	vec2 q = vec2( PREFIX(fbm)( p + dx1 ), PREFIX(fbm)( p + dy1 ) );
	vec2 r = vec2( PREFIX(fbm)( p + 1.5 * q + dx2 ), PREFIX(fbm)( p + 1.5 * q + dy2 ) );
	vec2 s = vec2( PREFIX(fbm)( p + 1.5 * r + dx1 + dx2 ), PREFIX(fbm)( p + 1.5 * r + dy2 + dy2 ) );

	float v = PREFIX(fbm)( p + 4. * s );
	vec3 col = v * vec3(q.x, r.x, s.x) + vec3(q.y, r.y, s.y);
	return vec4( col, 1. );
}
