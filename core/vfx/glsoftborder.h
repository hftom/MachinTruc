#ifndef GLSOFTBORDER_H
#define GLSOFTBORDER_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "glfilter.h"



class MySoftBorderEffect : public Effect {
public:
	MySoftBorderEffect();
	virtual std::string effect_type_id() const { return "MySoftBorderEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("soft_border.frag"); }
	
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
	Q_OBJECT
public:
	GLSoftBorder( QString id, QString name );
	
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *borderSize;
};

#endif // GLSOFTBORDER_H
