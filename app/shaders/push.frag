vec4 PREFIX(in1)( vec2 tc ) {
	vec4 sum = vec4(0.0);
	for ( float i = 0.0; i < PREFIX(loop); ++i ) {
		vec2 coord = tc + ( i * PREFIX(texSize) );
#if VERTICAL
#if DIRECTION
		if ( coord.y < 0.0 )
			sum += INPUT2( vec2( coord.x, coord.y + 1.0 ) );
#else
		if ( coord.y > 1.0 )
			sum += INPUT2( vec2( coord.x, coord.y - 1.0 ) );
#endif
#else
#if DIRECTION
		if ( coord.x > 1.0 )
			sum += INPUT2( vec2( coord.x - 1.0, coord.y ) );
#else
		if ( coord.x < 0.0 )
			sum += INPUT2( vec2( coord.x + 1.0, coord.y ) );
#endif
#endif
		else
			sum += INPUT1( coord );
	}
	return sum / PREFIX(loop);
}

vec4 PREFIX(in2)( vec2 tc ) {
	vec4 sum = vec4(0.0);
	for ( float i = 0.0; i < PREFIX(loop); ++i ) {
		vec2 coord = tc + ( i * PREFIX(texSize) );
#if VERTICAL
#if DIRECTION
		if ( coord.y > 1.0 )
			sum += INPUT1( vec2( coord.x, coord.y - 1.0 ) );
#else
		if ( coord.y < 0.0 )
			sum += INPUT1( vec2( coord.x, coord.y + 1.0 ) );
#endif
#else
#if DIRECTION
		if ( coord.x < 0.0 )
			sum += INPUT1( vec2( coord.x + 1.0, coord.y ) );
#else
		if ( coord.x > 1.0 )
			sum += INPUT1( vec2( coord.x - 1.0, coord.y ) );
#endif
#endif
		else
			sum += INPUT2( coord );
	}
	return sum / PREFIX(loop);
}

vec4 FUNCNAME( vec2 tc ) {
#if VERTICAL
#if DIRECTION
	if ( tc.y >= PREFIX(position) )
		return PREFIX(in1)( tc - vec2( 0.0, PREFIX(position) ) );
	return PREFIX(in2)( tc + vec2( 0.0, 1.0 - PREFIX(position) ) );
#else
	if ( tc.y >= 1.0 - PREFIX(position) )
		return PREFIX(in2)( tc - vec2( 0.0, 1.0 - PREFIX(position) ) );
	return PREFIX(in1)( tc + vec2( 0.0, PREFIX(position) ) );
#endif
#else
#if DIRECTION
	if ( tc.x >= 1.0 - PREFIX(position) )
		return PREFIX(in2)( tc - vec2( 1.0 - PREFIX(position), 0.0 ) );
	return PREFIX(in1)( tc + vec2( PREFIX(position), 0.0 ) );
#else
	if ( tc.x >= PREFIX(position) )
		return PREFIX(in1)( tc - vec2( PREFIX(position), 0.0 ) );
	return PREFIX(in2)( tc + vec2( 1.0 - PREFIX(position), 0.0 ) );
#endif
#endif
#undef VERTICAL
#undef DIRECTION
}
