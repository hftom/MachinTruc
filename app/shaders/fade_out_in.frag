vec4 FUNCNAME( vec2 tc ) {
	if ( PREFIX(show_second) > 0.0 )
		return INPUT2( tc ) * PREFIX(opacity);
	return INPUT1( tc ) * PREFIX(opacity);
}
