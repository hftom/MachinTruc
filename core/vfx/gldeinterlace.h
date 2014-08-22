#ifndef GLDEINTERLACE_H
#define GLDEINTERLACE_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



static const char *MyDeinterlaceEffect_frag=
"uniform float PREFIX(one_div_height);\n"
"vec4 FUNCNAME( vec2 tc ) {\n"
"	vec4 blend = 2.0 * INPUT( tc );\n"
"	blend += INPUT( vec2( tc.x, tc.y - PREFIX(one_div_height) ) );\n"
"	blend += INPUT( vec2( tc.x, tc.y + PREFIX(one_div_height) ) );\n"
"	return blend / 4.0;\n"
"}\n";



class MyDeinterlaceEffect : public Effect {
public:
	MyDeinterlaceEffect() {}
	virtual std::string effect_type_id() const { return "MyDeinterlaceEffect"; }
	std::string output_fragment_shader() { return MyDeinterlaceEffect_frag; }
	virtual bool needs_texture_bounce() const { return true; }
	virtual AlphaHandling alpha_handling() const { return DONT_CARE_ALPHA_TYPE; }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		set_uniform_float( glsl_program_num, prefix, "one_div_height", 1.0f / iheight );
	}
	
private:
	float iwidth, iheight;
};



class GLDeinterlace : public GLFilter
{
public:
	GLDeinterlace( QString id = "DeinterlaceAuto", QString name = "DeinterlaceAuto" );
	~GLDeinterlace();

	QList<Effect*> getMovitEffects();
};

#endif // GLDEINTERLACE_H
