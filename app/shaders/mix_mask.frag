vec4 FUNCNAME(vec2 tc) {
	vec4 org = INPUT1(tc);
	vec4 effect = INPUT2(tc);
#if INVERT
	float mask = 1.0 - INPUT3(tc).a;
#else
	float mask = INPUT3(tc).a;
#endif

#if SHOW
	return vec4(1.0) * mask + vec4(0.0, 0.0, 0.0, 1.0 - mask);
#else
	return vec4(mix( org, effect, mask ));
#endif
#undef INVERT
#undef SHOW
}
