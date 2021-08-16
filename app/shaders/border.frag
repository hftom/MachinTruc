uniform vec2 PREFIX(border);
vec4 FUNCNAME(vec2 tc) {
	if ( any( lessThan( tc, PREFIX(border) ) ) ||
		any( greaterThan( tc, 1.0 - PREFIX(border) ) ) ) {
		return PREFIX(color);
	}

	return INPUT(tc);
}
