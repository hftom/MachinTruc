#ifndef GLROTATE_H
#define GLROTATE_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "vfx/glfilter.h"



static const char *MyRotateEffect_shader=
"uniform vec2 PREFIX(center);\n"
"uniform vec2 PREFIX(factor);\n"
"uniform vec2 PREFIX(cosSin);\n"
"\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	tc -= PREFIX(center);\n"
"	tc *= PREFIX(factor);\n"
"	tc *= mat2( PREFIX(cosSin).x, PREFIX(cosSin).y, -PREFIX(cosSin).y, PREFIX(cosSin).x );\n"
"	tc /= PREFIX(factor);\n"
"	tc += PREFIX(center);\n"
"	return INPUT( tc );\n"
"}\n";



class MyRotateEffect : public Effect {
public:
	MyRotateEffect();
	virtual std::string effect_type_id() const { return "MyRotateEffect"; }
	std::string output_fragment_shader() { return MyRotateEffect_shader; }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		//Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		Q_UNUSED( sampler_num );
		float center[2] = { 0.5f, 0.5f };
		set_uniform_vec2( glsl_program_num, prefix, "center", center );
		
		// we have to take both texture_size_ratio and sample_aspect_ratio into account.
		float factor[2] = { SAR * iwidth / iheight, 1.0f };
		set_uniform_vec2( glsl_program_num, prefix, "factor", factor );
		
		float cosSin[2] = { (float)cos( angle ),  (float)sin( angle ) };
		set_uniform_vec2( glsl_program_num, prefix, "cosSin", cosSin );
	}

private:
	float iwidth, iheight;
	float angle;
	float SAR;
};



class GLRotate : public GLFilter
{
public:
	GLRotate( QString id, QString name );
	~GLRotate();
	
	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *angle;
};

#endif // GLROTATE_H
