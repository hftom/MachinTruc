vec4 FUNCNAME(vec2 tc) {
	tc.y = 1.0 - tc.y;
	return INPUT(tc);
}
