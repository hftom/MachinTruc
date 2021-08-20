vec4 FUNCNAME( vec2 tc ) {
	if ( tc.x > PREFIX(position) && tc.x < 1.0 - PREFIX(position) ) {
		if ( PREFIX(strength_first) > 0.5 )
			return INPUT1(tc);
		return INPUT2(tc);
	}
	vec4 first = INPUT1(tc);
	vec4 second = INPUT2(tc);
	vec4 result = vec4(PREFIX(strength_first)) * first + vec4(PREFIX(strength_second)) * second;
	result.a = clamp(result.a, 0.0, 1.0);
	return result;
}
