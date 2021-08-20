vec4 FUNCNAME( vec2 tc ) {
	tc -= 0.5;
	float vertColor = 0.0;
	float t = PREFIX(time) * 0.5;
	for( float i = 0.0; i < 4.0; i++ ) {
		tc.y += sin( t + tc.x * 6.0 ) * 0.45;
		tc.x += sin( -t + tc.y * 3.0 ) * 0.25;
		float value = atan( tc.y * 2.5 ) + sin( tc.x * 10.0 );
		float stripColor = 1.0 / sqrt( abs( value ) );
		vertColor += stripColor / 14.0 / 4.0;
	}
	return clamp( vec4( vec3( vertColor * 0.7, vertColor / 15.0, vertColor / 20.0 ), 1.0 ), vec4(0.0), vec4(1.0) );
}
