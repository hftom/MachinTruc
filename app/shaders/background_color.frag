vec4 FUNCNAME(vec2 tc) {
	vec4 top = INPUT(tc);
	return top + (1.0 - top.a) * PREFIX(color);
}
