#ifndef GLSOFTBORDER_H
#define GLSOFTBORDER_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "glfilter.h"

static const char *MySoftBorderEffect_shader=
"uniform vec2 PREFIX(border);\n"
"uniform vec2 PREFIX(half_texel);\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	float a = 1.0;\n"
"	if ( any( greaterThan( PREFIX(border), vec2(0.0) ) ) ) {\n"
"		vec2 coord = tc - PREFIX(half_texel);\n"
"		a = coord.x / PREFIX(border.x);\n"
"		a = min( a, coord.y / PREFIX(border.y) );\n"
"		coord = tc + PREFIX(half_texel);\n"
"		a = min( a, ( 1.0 - coord.x ) / PREFIX(border.x) );\n"
"		a = min( a, ( 1.0 - coord.y ) / PREFIX(border.y) );\n"
"	}\n"
"	return INPUT( tc ) * clamp( a, 0.0, 1.0 );\n"
"}\n";



class MySoftBorderEffect : public Effect {
public:
	MySoftBorderEffect();
	virtual std::string effect_type_id() const { return "MySoftBorderEffect"; }
	std::string output_fragment_shader() { return MySoftBorderEffect_shader; }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Q_UNUSED( sampler_num );
		float border[2] = { 1.0f / iwidth * borderSize, 1.0f / iheight * borderSize };
		set_uniform_vec2( glsl_program_num, prefix, "border", border );
		float half_texel[2] = { 1.0f / iwidth / 2.0f, 1.0f / iheight / 2.0f };
		set_uniform_vec2( glsl_program_num, prefix, "half_texel", half_texel );
	}

private:
	float iwidth, iheight;
	float borderSize;
};



class GLSoftBorder : public GLFilter
{
public:
	GLSoftBorder( QString id, QString name );
	
	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *borderSize;
};

#endif // GLSOFTBORDER_H
