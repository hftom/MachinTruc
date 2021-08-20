uniform vec2 PREFIX(pixsize);
vec4 FUNCNAME(vec2 tc) {
	float x = tc.x / PREFIX(pixsize).x;
	tc.x = PREFIX(pixsize).x * int(x);
	float y = tc.y / PREFIX(pixsize).y;
	tc.y = PREFIX(pixsize).y * int(y);
	return INPUT( tc );
}
