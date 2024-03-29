#ifndef GLDEFISH_H
#define GLDEFISH_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "glfilter.h"



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
	std::string output_fragment_shader() { return GLFilter::getShader("defish.frag"); }
	
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
	Q_OBJECT
public:
	GLDefish( QString id, QString name );
	
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *amount;
	Parameter *scale;
};

#endif // GLDEFISH_H
