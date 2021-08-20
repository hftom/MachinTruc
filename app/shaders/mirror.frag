vec4 FUNCNAME( vec2 tc ) {
	if (PREFIX(horizontal) > 0.0)
		tc.x = 1.0 - tc.x;
	if (PREFIX(vertical) > 0.0)
		tc.y = 1.0 - tc.y;
	return INPUT( tc );
}
