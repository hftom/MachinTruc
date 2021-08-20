vec4 FUNCNAME( vec2 tc ) {
	float col = fract( sin( dot((tc + 0.007 * fract(PREFIX(time))) ,vec2(12.9898,78.233)) ) * 43758.5453 );
	return vec4( col, col, col, 1.0 );
}
