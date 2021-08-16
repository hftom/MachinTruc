uniform vec2 PREFIX(factor);
uniform vec2 PREFIX(cosSin);
uniform vec2 PREFIX(offset);

vec4 FUNCNAME( vec2 tc ) {
	tc -= 0.5;
	tc *= PREFIX(factor);
	tc *= mat2( PREFIX(cosSin).x, PREFIX(cosSin).y, -PREFIX(cosSin).y, PREFIX(cosSin).x );
	tc /= PREFIX(factor);
	tc -= PREFIX(offset);
	tc += 0.5;
	return clamp( INPUT( tc ), vec4(0.0), vec4(1.0) );
}
