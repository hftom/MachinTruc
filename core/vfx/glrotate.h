#ifndef GLROTATE_H
#define GLROTATE_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "vfx/glfilter.h"



static const char *MyRotateEffect_shader=
"uniform vec2 PREFIX(center);\n"
"uniform vec2 PREFIX(texSize);\n"
"uniform vec2 PREFIX(cosSin);\n"
"uniform vec2 PREFIX(border);\n"
"uniform vec2 PREFIX(half_texel);\n"
"\n"
"vec4 PREFIX(AAborder)( vec2 tc ) {\n"
"	vec2 coord = tc - PREFIX(half_texel);\n"
"	float a = coord.x / PREFIX(border.x);\n"
"	a = min( a, coord.y / PREFIX(border.y) );\n"
"	coord = tc + PREFIX(half_texel);\n"
"	a = min( a, ( 1.0 - coord.x ) / PREFIX(border.x) );\n"
"	a = min( a, ( 1.0 - coord.y ) / PREFIX(border.y) );\n"
"	return INPUT( tc ) * clamp( a, 0.0, 1.0 );\n"
"}\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	tc -= PREFIX(center);\n"
"	tc *= PREFIX(texSize);\n"
"	tc *= mat2( PREFIX(cosSin).x, PREFIX(cosSin).y, -PREFIX(cosSin).y, PREFIX(cosSin).x );\n"
"	tc /= PREFIX(texSize);\n"
"	tc += PREFIX(center);\n"
"	return PREFIX(AAborder)( tc );\n"
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
		float half_texel[2] = { 1.0f / iwidth / 2.0f, 1.0f / iheight / 2.0f };
		set_uniform_vec2( glsl_program_num, prefix, "half_texel", half_texel );
		float border[2] = { 1.0f / iwidth * (float)aaBorder, 1.0f / iheight * (float)aaBorder };
		set_uniform_vec2( glsl_program_num, prefix, "border", border );
		float center[2] = { 0.5f, 0.5f };
		set_uniform_vec2( glsl_program_num, prefix, "center", center );
		float texSize[2] = { iwidth, iheight };
		set_uniform_vec2( glsl_program_num, prefix, "texSize", texSize );
		float cosSin[2] = { (float)cos( angle ),  (float)sin( angle ) };
		set_uniform_vec2( glsl_program_num, prefix, "cosSin", cosSin );
	}

private:
	float iwidth, iheight;
	float angle;
	int aaBorder;
};



class GLRotate : public GLFilter
{
public:
	GLRotate( QString id, QString name );
	~GLRotate();
	
	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *angle, *aaBorder;
};

#endif // GLROTATE_H
