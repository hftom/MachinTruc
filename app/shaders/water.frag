/* Simple Water shader. (c) Victor Korsun, bitekas@gmail.com; 2012.
   Attribution-ShareAlike CC License.
*/
uniform vec2 PREFIX(size_div_delta);

// crystals effect
const float PREFIX(delta_theta) = 2.0 * 3.1415926535897932 / 7.0;
float PREFIX(color)( vec2 coord ) {
	float col = 0.0;
	float theta = 0.0;
	for ( int i = 0; i < 8; i++ ) {
		vec2 adjc = coord;
		theta = PREFIX(delta_theta) * float( i );
		adjc.x += cos( theta ) * PREFIX(time) * PREFIX(speed);
		adjc.y -= sin( theta ) * PREFIX(time) * PREFIX(speed);
		col = col + cos( ( adjc.x * cos( theta ) - adjc.y * sin( theta ) ) * PREFIX(frequency) ) * PREFIX(intensity);
	}
	return cos( col );
}

vec4 FUNCNAME(vec2 tc) {
	vec2 p = tc, c1 = p, c2 = p;
	float cc1 = PREFIX(color)( c1 );
	c2.x += PREFIX(size_div_delta).x;
	float dx = PREFIX(emboss) * ( cc1 - PREFIX(color)( c2 ) ) / PREFIX(delta);
	c2.x = p.x;
	c2.y -= PREFIX(size_div_delta).y;
	float dy = PREFIX(emboss) * ( cc1 - PREFIX(color)( c2 ) ) / PREFIX(delta);
	c1.x += dx;
	c1.y += dy;
	float alpha = 1.0 + dot( dx, dy ) * PREFIX(intence);
	vec4 result = INPUT( c1 );
	result.rgb *= alpha;
	return result;
}

/*
// noise
float PREFIX(rand)(vec2 seed){
	return fract(sin(dot(seed ,vec2(12.9898,78.233))) * 43758.5453) - 0.5;
}
*/
