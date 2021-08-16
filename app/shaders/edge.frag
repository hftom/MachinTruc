uniform vec2 PREFIX(one_div_size);
vec4 FUNCNAME( vec2 tc ) {
	float xof = PREFIX(one_div_size).x;
	float yof = PREFIX(one_div_size).y;
	vec4 c1 = INPUT( tc );
	vec4 c2 = INPUT( vec2( tc.x + xof, tc.y ) );
	float diff = abs(c2.r - c1.r) + abs(c2.g - c1.g) + abs(c2.b - c1.b);
	c2 = INPUT( vec2( tc.x, tc.y + yof ) );
	diff += abs(c2.r - c1.r) + abs(c2.g - c1.g) + abs(c2.b - c1.b);
	c2 = INPUT( vec2( tc.x - xof, tc.y ) );
	diff += abs(c2.r - c1.r) + abs(c2.g - c1.g) + abs(c2.b - c1.b);
	c2 = INPUT( vec2( tc.x, tc.y - yof ) );
	diff += abs(c2.r - c1.r) + abs(c2.g - c1.g) + abs(c2.b - c1.b);
	diff *= PREFIX(amp);
	if ( diff < PREFIX(depth) )
		diff = 0.0;
	diff = clamp( diff, 0.0, 1.0 );
	vec4 top = vec4( vec3( 0.0 ), diff ) * c1.a;
	vec4 bottom = mix( c1, vec4(1.0, 1.0, 1.0, c1.a), PREFIX(opacity));
	return (top + (1.0 - top.a) * bottom) * c1.a;
}
