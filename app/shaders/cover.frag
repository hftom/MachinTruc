vec4 PREFIX(in2)( vec2 tc ) {
	vec4 sum = vec4(0.0);
	for ( float i = 0.0; i < PREFIX(loop); ++i ) {
		vec2 coord = tc + ( i * PREFIX(texSize) );
#if VERTICAL
#if DIRECTION
		if ( coord.y > 1.0 + (PREFIX(loop) * PREFIX(texSize).y) )
			sum += SOURCE1( vec2( tc.x, PREFIX(position) - 1.0 + tc.y ) );
#else
		if ( coord.y < (PREFIX(loop) * PREFIX(texSize).y) )
			sum += SOURCE1( vec2( tc.x, 1.0 - PREFIX(position) + tc.y ) );
#endif
#else
#if DIRECTION
		if ( coord.x < (PREFIX(loop) * PREFIX(texSize).x) )
			sum += SOURCE1( vec2( 1.0 - PREFIX(position) + tc.x, tc.y ) );
#else
		if ( coord.x > 1.0 + (PREFIX(loop) * PREFIX(texSize).x) )
			sum += SOURCE1( vec2( PREFIX(position) - 1.0 + tc.x, tc.y ) );
#endif
#endif
		else
			sum += SOURCE2( coord );
	}
	return sum / PREFIX(loop);
}

vec4 FUNCNAME( vec2 tc ) {
#if VERTICAL
#if DIRECTION
	if ( tc.y >= PREFIX(position) )
		return SOURCE1( tc );
	return PREFIX(in2)( tc + vec2( 0.0, 1.0 - PREFIX(position) ) );
#else
	if ( tc.y >= 1.0 - PREFIX(position) )
		return PREFIX(in2)( tc - vec2( 0.0, 1.0 - PREFIX(position) ) );
	return SOURCE1( tc );
#endif
#else
#if DIRECTION
	if ( tc.x >= 1.0 - PREFIX(position) )
		return PREFIX(in2)( tc - vec2( 1.0 - PREFIX(position), 0.0 ) );
	return SOURCE1( tc );
#else
	if ( tc.x >= PREFIX(position) )
		return SOURCE1( tc );
	return PREFIX(in2)( tc + vec2( 1.0 - PREFIX(position), 0.0 ) );
#endif
#endif
#undef VERTICAL
#undef DIRECTION
#undef SOURCE1
#undef SOURCE2
}
