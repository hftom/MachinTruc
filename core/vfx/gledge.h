#ifndef GLEDGE_H
#define GLEDGE_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



class EdgeEffect : public Effect {
public:
	EdgeEffect() : amp(3.0), depth(1.0), opacity(1.0), iwidth(1), iheight(1) {
		register_float("amp", &amp);
		register_float("depth", &depth);
		register_float("opacity", &opacity);
	}
	std::string effect_type_id() const { return "EdgeEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("edge.frag"); }
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
	float amp;
	float depth;
	float opacity;
	float iwidth, iheight;
};



class GLEdge : public GLFilter
{
	Q_OBJECT
public:
	GLEdge( QString id, QString name );
	~GLEdge();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *amp, *depth, *opacity, *blur;
};

#endif // GLEDGE_H
