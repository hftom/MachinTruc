vec4 FUNCNAME( vec2 tc ) {
	if ( PREFIX(show_second) > 0.0 )
		return INPUT2( tc );
	return INPUT1( tc );
}
