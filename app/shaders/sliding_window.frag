vec4 FUNCNAME( vec2 tc ) {
	if ( tc.x <= PREFIX(position) || tc.x >= 1.0 - PREFIX(position) )
		return INPUT2( tc );
	return INPUT1( tc );
}
