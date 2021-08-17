vec4 FUNCNAME(vec2 tc) {
	vec3 hsv = PREFIX(rgb2hsv)(INPUT(tc).rgb, PREFIX(hsvColor).x + 180.0);
	float offset = 180.0 - PREFIX(hsvColor).x;
	hsv.x = mod(hsv.x + offset, 360.0);
	float hx = PREFIX(hsvColor).x + offset;

	float d = distance(hsv.x, hx) / PREFIX(varianceH);
	d += distance(hsv.y, PREFIX(hsvColor).y) / PREFIX(varianceS);
	d += distance(hsv.z, PREFIX(hsvColor).z) / PREFIX(varianceV);
	d = min(d / 3.0, 1.0);
	if (d <= 0.5) {
		d = pow(2 * d, PREFIX(gain)) / 2.0;
	}
	else {
		d = -(pow(2 * (1.0 - d), PREFIX(gain)) / 2.0) + 1.0;
	}
	return vec4(1.0 - d);
}
