uniform float PREFIX(radius);
uniform vec2 PREFIX(size);
uniform vec2 PREFIX(half_size);
vec4 FUNCNAME(vec2 tc) {
	vec2 new = tc * PREFIX(size) - PREFIX(half_size);
	float r = length( new ) / PREFIX(radius);
	float theta = 1.0;
	if  ( r != 0.0 )
		theta = atan( r ) / r;

	return INPUT( (theta * new * PREFIX(scale) + PREFIX(half_size)) / PREFIX(size) );
}
