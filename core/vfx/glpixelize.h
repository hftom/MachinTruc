#ifndef GLPIXELIZE_H
#define GLPIXELIZE_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "vfx/glmask.h"



class MyPixelizeEffect : public Effect {
public:
	MyPixelizeEffect();
	virtual std::string effect_type_id() const { return "MyPixelizeEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("pixelize.frag"); }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		float pixsize[2] = { pixelSize / iwidth, pixelSize / iheight };
		set_uniform_vec2( glsl_program_num, prefix, "pixsize", pixsize );
	}

private:
	float iwidth, iheight;
	float pixelSize;
};



class GLPixelize : public GLMask
{
	Q_OBJECT
public:
	GLPixelize( QString id, QString name );
	~GLPixelize();
	
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	QString getDescriptor( double pts, Frame *src, Profile *p  );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *pixelSize;
};

#endif // GLPIXELIZE_H
