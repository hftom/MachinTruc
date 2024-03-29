#ifndef GLBORDER_H
#define GLBORDER_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "glfilter.h"



class MyBorderEffect : public Effect {
public:
	MyBorderEffect();
	virtual std::string effect_type_id() const { return "MyBorderEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("border.frag"); }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		float border[2] = { 1.0f / iwidth * borderSize, 1.0f / iheight * borderSize };
		set_uniform_vec2( glsl_program_num, prefix, "border", border );
	}

private:
	float iwidth, iheight;
	float borderSize;
	RGBATuple color;
};



class GLBorder : public GLFilter
{
	Q_OBJECT
public:
	GLBorder( QString id, QString name );
	
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *borderSize, *color;
};

#endif // GLBORDER_H
