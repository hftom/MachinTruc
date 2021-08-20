#ifndef GLWATER_H
#define GLWATER_H

#include <movit/effect.h>
#include <movit/effect_util.h>

#include "vfx/glfilter.h"



class WaterEffect : public Effect {
public:
	WaterEffect() 
		: time(0.0),
		inwidth(1280),
		inheight(720),
		speed(0.1),
		emboss(0.4),
		intensity(3.0),
		frequency(4.0),
		delta(220.0),
		intence(700.0)
	{
		register_float( "time", &time );
		register_float( "speed", &speed );
		register_float( "emboss", &emboss );
		register_float( "intensity", &intensity );
		register_float( "frequency", &frequency );
		register_float( "delta", &delta );
		register_float( "intence", &intence );
	}

	virtual std::string effect_type_id() const { return "WaterEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("water.frag"); }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		inwidth = width;
		inheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		float size_div_delta[2] = { inwidth / delta, inheight / delta };
		set_uniform_vec2( glsl_program_num, prefix, "size_div_delta", size_div_delta );
	}

private:
	float time, inwidth, inheight;
	//speed
	float speed;
	// refraction
	float emboss, intensity, frequency;
	// reflection
	float delta, intence;
};



class GLWater : public GLFilter
{
	Q_OBJECT
public:
	GLWater( QString id, QString name );
	~GLWater();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *speed;
	Parameter *emboss, *intensity, *frequency;
	Parameter *delta, *intence;
};

#endif //GLWATER_H
