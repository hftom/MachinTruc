vec4 FUNCNAME( vec2 tc ) {
	vec4 col = INPUT( tc );
	vec3 c = col.rgb / col.a;
	c = pow(c, vec3(1.0 / PREFIX(brightness)));
	c = mix( c, vec3(0.5),  -PREFIX(contrast));
	return vec4( clamp( c, 0.0, 1.0 ) * col.a, col.a );
}
