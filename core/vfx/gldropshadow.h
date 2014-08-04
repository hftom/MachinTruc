#ifndef GLDROPSHADOW_H
#define GLDROPSHADOW_H

#include <movit/blur_effect.h>
#include <movit/overlay_effect.h>
#include <movit/effect_util.h>
#include <movit/util.h>

#include "vfx/glfilter.h"



static const char *MyShadowMapEffect_frag=
"uniform vec2 PREFIX(offset);\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	float a = INPUT( tc - PREFIX(offset) ).a;\n"
"	return vec4(0.0, 0.0, 0.0, PREFIX(opacity)) * a;\n"
"}\n";



class MyShadowMapEffect : public Effect {
public:
	MyShadowMapEffect();
	virtual std::string effect_type_id() const { return "MyShadowMapEffect"; }
	std::string output_fragment_shader() { return MyShadowMapEffect_frag; }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		float offset[2] = { xoffset / iwidth, -yoffset / iheight };
		set_uniform_vec2( glsl_program_num, prefix, "offset", offset );
	}

private:
	float iwidth, iheight;
	float xoffset, yoffset;
	float opacity;
};



class MyDropShadowEffect : public Effect {
public:
	MyDropShadowEffect();
	virtual std::string effect_type_id() const { return "MyDropShadowEffect"; }

	virtual void rewrite_graph(EffectChain *graph, Node *self);
	virtual bool set_float(const std::string &key, float value);

	virtual std::string output_fragment_shader() {
		assert(false);
	}
	virtual void set_gl_state(GLuint, const std::string&, unsigned*) {
		assert(false);
	}

private:
	BlurEffect *blur;
	MyShadowMapEffect *shadow;
	OverlayEffect *overlay;
};



class GLDropShadow : public GLFilter
{
public:
    GLDropShadow( QString id, QString name );
    ~GLDropShadow();
	
	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *radius, *opacity, *xoffset, *yoffset;
};

#endif // GLDROPSHADOW_H
