uniform sampler2D PREFIX(string_tex);
uniform vec2 PREFIX(offset);
uniform vec2 PREFIX(scale);

vec4 FUNCNAME( vec2 tc ) {
	vec4 background = INPUT( tc );
	tc -= PREFIX(offset);
	tc *= PREFIX(scale);
	vec4 text = tex2D( PREFIX(string_tex), vec2( tc.x, 1.0 - tc.y ) ) * PREFIX(opacity);
	return text + (1.0 - text.a) * background;
}
