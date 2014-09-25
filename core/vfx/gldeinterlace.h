#ifndef GLDEINTERLACE_H
#define GLDEINTERLACE_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



static const char *MyDeinterlaceEffect_frag=
"uniform vec2 PREFIX(one_div_size);\n"
"vec4 FUNCNAME( vec2 tc ) {\n"
"	float parity = mod( floor( tc.y / PREFIX(one_div_size).y ), 2.0 );\n"
"\n"
"	// Keep even lines and interpolate odd ones.\n"
"	// Movit flips y, so parity is inversed.\n"
"	if ( parity == 0.0 ) {\n"
"		vec4 ret = INPUT( vec2( tc.x, tc.y - PREFIX(one_div_size).y ) );\n"
"		ret += INPUT( vec2( tc.x, tc.y + PREFIX(one_div_size).y ) );\n"
"		return INPUT( vec2( tc.x, tc.y - PREFIX(one_div_size).y ) );//ret / 2.0;\n"
"	}\n"
"	return INPUT( tc );\n"
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
	
	virtual void inform_added( EffectChain *chain ) { this->chain = chain; }
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		float one_div_size[2] = { 1.0f / iwidth, 1.0f / iheight };
		set_uniform_vec2( glsl_program_num, prefix, "one_div_size", one_div_size );
	}
	
private:
	float iwidth, iheight;
	EffectChain *chain;
};



class GLDeinterlace : public GLFilter
{
public:
	GLDeinterlace( QString id = "DeinterlaceAuto", QString name = "DeinterlaceAuto" );
	~GLDeinterlace();

	QList<Effect*> getMovitEffects();
};

#endif // GLDEINTERLACE_H
