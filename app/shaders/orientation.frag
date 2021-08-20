vec4 FUNCNAME( vec2 tc ) {
#ifdef ANGLE_90
	return INPUT( vec2(1.0 - tc.y, tc.x) );
#endif
#ifdef ANGLE_270
	return INPUT( vec2(tc.y, 1.0 - tc.x) );
#endif
	return INPUT( vec2(1.0 - tc.x, 1.0 - tc.y) );
#undef ANGLE_90
#undef ANGLE_270
}
