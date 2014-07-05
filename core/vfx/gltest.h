#ifndef GLTEST_H
#define GLTEST_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



static const char *MTestEffect_frag=
"uniform vec2 PREFIX(one_div_size);\n"
"vec4 FUNCNAME( vec2 tc ) {\n"
"	vec4 sum = INPUT( tc );\n"
"	for ( int i = 1; i < int(PREFIX(loop)); ++i ) {\n"
"		sum += INPUT( tc + ( PREFIX(one_div_size) * float(i) ) );\n"
"		sum += INPUT( tc - ( PREFIX(one_div_size) * float(i) ) );\n"
"	}\n"
"	return  sum / ( 1.0 + ((PREFIX(loop) - 1.0) * 2.0) );\n"
"}\n";



class MTestEffect : public Effect {
public:
	MTestEffect() : loop(1.0) {
		register_float("loop", &loop);
	}
	std::string effect_type_id() const { return "MTestEffect"; }
	std::string output_fragment_shader() { return MTestEffect_frag; }
	bool needs_texture_bounce() const { return true; }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		float size[2] = { 1.0f / iwidth, 1.0f / iheight };
		set_uniform_vec2( glsl_program_num, prefix, "one_div_size", size );
	}
	
private:
	float loop;
	float iwidth, iheight;
};



class GLTest : public GLFilter
{
public:
    GLTest( QString id, QString name );
    ~GLTest();

    bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	float loop;
};

#endif // GLTEST_H
