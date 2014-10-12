#ifndef INPUT_COLOR_H
#define INPUT_COLOR_H

#include <movit/input.h>

using namespace movit;




static const char *inputColorShader=
"vec4 FUNCNAME(vec2 tc) {\n"
"	return vec4( 0.0, 0.0, 0.0, 1.0 );\n"
"}\n";



class InputColor : public Input
{
public:
	InputColor() : needs_mipmaps( false ) {
		register_int("needs_mipmaps", &needs_mipmaps);
	}
	
	std::string effect_type_id() const { return "InputColor"; }
	
	bool can_output_linear_gamma() const { return true; }
	unsigned get_width() const { return 1920; }
	unsigned get_height() const {return 1080; }
	Colorspace get_color_space() const { return COLORSPACE_sRGB; }
	GammaCurve get_gamma_curve() const { return GAMMA_LINEAR; }
	
	std::string output_fragment_shader() { return inputColorShader; }
	
private:
	int needs_mipmaps;
};
#endif // INPUT_COLOR_H