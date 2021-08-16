/* based on work by florian berger (flockaroo) - 2016
* License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
*/
uniform vec2 PREFIX(pixelSize);

float PREFIX(gray)(vec2 tc) {
	return dot(INPUT(tc).rgb, vec3(1.0 / 3.0));
}

vec4 FUNCNAME(vec2 tc) {
	vec2 offset = vec2(1.0, 0.0);
	return vec4(
		PREFIX(gray)(tc + PREFIX(pixelSize) * offset.xy) - PREFIX(gray)(tc - PREFIX(pixelSize) * offset.xy),
		PREFIX(gray)(tc + PREFIX(pixelSize) * offset.yx) - PREFIX(gray)(tc - PREFIX(pixelSize) * offset.yx),
		0.0, 0.0
	) / 2.0;
}
