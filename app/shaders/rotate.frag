uniform vec2 PREFIX(factor);
uniform vec2 PREFIX(cosSin);
uniform vec2 PREFIX(offset);
uniform vec2 PREFIX(scale);
uniform vec2 PREFIX(centerOffset);

vec4 FUNCNAME( vec2 tc ) {
	tc -= PREFIX(offset);
	tc *= PREFIX(scale);
	tc -= PREFIX(centerOffset);
	tc *= PREFIX(factor);
	tc *= mat2( PREFIX(cosSin).x, PREFIX(cosSin).y, -PREFIX(cosSin).y, PREFIX(cosSin).x );
	tc /= PREFIX(factor);
	tc += PREFIX(centerOffset);
	return clamp( INPUT( tc ), vec4(0.0), vec4(1.0) );
}
