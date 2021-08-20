uniform vec2 PREFIX(border);
uniform vec2 PREFIX(half_texel);
vec4 FUNCNAME(vec2 tc) {
	float a = 1.0;
	if ( any( greaterThan( PREFIX(border), vec2(0.0) ) ) ) {
		vec2 coord = tc - PREFIX(half_texel);
		a = coord.x / PREFIX(border.x);
		a = min( a, coord.y / PREFIX(border.y) );
		coord = tc + PREFIX(half_texel);
		a = min( a, ( 1.0 - coord.x ) / PREFIX(border.x) );
		a = min( a, ( 1.0 - coord.y ) / PREFIX(border.y) );
	}
	return INPUT( tc ) * clamp( a, 0.0, 1.0 );
}
