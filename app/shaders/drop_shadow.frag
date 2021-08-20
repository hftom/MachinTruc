uniform vec2 PREFIX(offset);
vec4 FUNCNAME(vec2 tc) {
	float a = INPUT( tc - PREFIX(offset) ).a;
	return vec4(PREFIX(color) * PREFIX(opacity), PREFIX(opacity)) * a;
}
