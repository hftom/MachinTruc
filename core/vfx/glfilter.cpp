#include "glfilter.h"



QString GLFilter::getShaderRGBTOHSV()
{
	return "vec3 PREFIX(rgb2hsv)(vec3 rgb, float defaultHue) {\n"
	"	vec3 hsv;\n"
	"	float mini = min(min(rgb.r, rgb.g), rgb.b);\n"
	"	float maxi = hsv.z = max(max(rgb.r, rgb.g), rgb.b);\n"
	"	float delta = maxi - mini;\n"
	"	if (maxi <= 0.0)\n"
	"		hsv.y = 0.0;\n"
	"	else\n"
	"		hsv.y = delta / maxi;\n"
	"	if (delta == 0.0) {\n"
	"		hsv.x = defaultHue;\n"
	"		return hsv;\n"
	"	}\n"
	"	if ( rgb.r == maxi )\n"
	"		hsv.x = ( rgb.g - rgb.b ) / delta;\n"
	"	else if ( rgb.g >= maxi )\n"
	"		hsv.x = 2.0 + ( rgb.b - rgb.r ) / delta;\n"
	"	else\n"
	"		hsv.x = 4.0 + ( rgb.r - rgb.g ) / delta;\n"
	"\n"
	"	hsv.x *= 60.0;\n"
	"	if ( hsv.x < 0.0 )\n"
	"		hsv.x += 360.0;\n"
	"\n"
	"	return hsv;\n"
	"}\n";
}
