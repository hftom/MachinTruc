#ifndef GLDEFISH_H
#define GLDEFISH_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "glfilter.h"



static const char *MyDefishEffect_shader=
"uniform float PREFIX(radius);\n"
"uniform vec2 PREFIX(size);\n"
"uniform vec2 PREFIX(half_size);\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	vec2 new = tc * PREFIX(size) - PREFIX(half_size);\n"
"	float r = length( new ) / PREFIX(radius);\n"
"	float theta = 1.0;\n"
"	if  ( r != 0.0 )\n"
"		theta = atan( r ) / r;\n"
"\n"
"	return INPUT( (theta * new * PREFIX(scale) + PREFIX(half_size)) / PREFIX(size) );\n"
"}\n";



class MyDefishEffect : public Effect {
public:
	MyDefishEffect()
		: iwidth( 1 ),
		iheight( 1 ),
		amount( 2.0 ),
		scale( 1.0 )
	{
		register_float("amount", &amount);
		register_float("scale", &scale);
	}
	
	virtual std::string effect_type_id() const { return "MyDefishEffect"; }
	std::string output_fragment_shader() { return MyDefishEffect_shader; }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		set_uniform_float( glsl_program_num, prefix, "radius", sqrt( iwidth * iwidth + iheight * iheight ) / amount );
		float size[2] = { iwidth, iheight };
		set_uniform_vec2( glsl_program_num, prefix, "size", size );
		float half_size[2] = { iwidth / 2.0f, iheight / 2.0f };
		set_uniform_vec2( glsl_program_num, prefix, "half_size", half_size );
	}

private:
	float iwidth, iheight;
	float amount, scale;
};



class GLDefish : public GLFilter
{
public:
	GLDefish( QString id, QString name );
	
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *amount;
	Parameter *scale;
};

#endif // GLDEFISH_H
