vec3 PREFIX(rgb2hsv)(vec3 rgb, float defaultHue) {
	vec3 hsv;
	float mini = min(min(rgb.r, rgb.g), rgb.b);
	float maxi = hsv.z = max(max(rgb.r, rgb.g), rgb.b);
	float delta = maxi - mini;
	if (maxi <= 0.0)
		hsv.y = 0.0;
	else
		hsv.y = delta / maxi;
	if (delta == 0.0) {
		hsv.x = defaultHue;
		return hsv;
	}
	if ( rgb.r == maxi )
		hsv.x = ( rgb.g - rgb.b ) / delta;
	else if ( rgb.g >= maxi )
		hsv.x = 2.0 + ( rgb.b - rgb.r ) / delta;
	else
		hsv.x = 4.0 + ( rgb.r - rgb.g ) / delta;

	hsv.x *= 60.0;
	if ( hsv.x < 0.0 )
		hsv.x += 360.0;

	return hsv;
}
