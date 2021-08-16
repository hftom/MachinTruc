/* based on work by florian berger (flockaroo) - 2016
* License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
*/
const float PREFIX(pi2) = 6.28318530717959;

vec4 PREFIX(getCol)(vec2 pos) {
	return INPUT1(pos / vec2(1920.0, 1080.0));
}

vec4 PREFIX(getColHT)(vec2 pos) {
	return PREFIX(getCol)(pos);//smoothstep(.95,1.05,PREFIX(getCol)(pos)*.8+.2+getRand(pos*.7));
}

vec4 FUNCNAME(vec2 tc) {
	float SampNum = 12.0;
	float stretch = 2.2;
	vec2 iResolution = vec2(1920.0, 1080.0);

	vec2 pos = (tc * iResolution);
	vec3 col = vec3(0);
	vec3 col2 = vec3(0);
	float sum = 0.0;
	for (float i = 0.0; i < 3.0; i++) {
		float ang = PREFIX(pi2) / 3.0 * (i + 0.8);
   	vec2 v = vec2(cos(ang), sin(ang));
		for (float j = 0.0; j < SampNum; j++) {
			vec2 dpos = v.yx * vec2(1.0, -1.0) * j * stretch;
			vec2 dpos2 = v.xy * j * j / SampNum * 0.5 * stretch;
			vec2 g;
			float fact;
			float fact2;
			for (float s = -1.0; s <= 1.0; s += 2.0) {
				vec2 pos2 = pos + s * dpos + dpos2;
				vec2 pos3 = pos + (s * dpos + dpos2).yx * vec2(1.0, -1.0) * 2.0;
				g = INPUT2(pos2 / vec2(1920.0, 1080.0)).rg;
				fact = dot(g, v) - 0.5 * abs(dot(g, v.yx * vec2(1.0, -1.0)));
				fact2 = dot(normalize(g + vec2(0.0001)), v.yx * vec2(1.0, -1.0));
				fact = clamp(fact, 0.0, 0.05);
				fact2 = abs(fact2);
				fact *= 1.0 - j / SampNum;
				col += fact;
				col2 += fact2 * PREFIX(getColHT)(pos3).xyz;
				sum += fact2;
			}
		}
	}
	col /= SampNum * 2.25 / sqrt(iResolution.y);
	col2 /= sum;
	//col.x *= (0.6 + 0.8 * getRand(pos * 0.7).x);
	col.x = 1.0 - col.x;
	col.x *= col.x * col.x;

	return vec4(vec3(col.x * col2), 1.0);
}
