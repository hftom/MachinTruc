#ifndef GLDENOISE_H
#define GLDENOISE_H

#include <movit/blur_effect.h>
#include <movit/effect_util.h>
#include <movit/util.h>

#include "vfx/glfilter.h"



static const char *DenoiseEdgeEffect_frag=
"uniform vec2 PREFIX(one_div_size);\n"
"vec4 FUNCNAME( vec2 tc ) {\n"
"	float xof = PREFIX(one_div_size).x;\n"
"	float yof = PREFIX(one_div_size).y;\n"
"	vec4 c1 = INPUT( tc );\n"
"	vec4 c2 = INPUT( vec2( tc.x + xof, tc.y ) );\n"
"	float diff = abs(c2.r - c1.r) + abs(c2.g - c1.g) + abs(c2.b - c1.b);\n"
"	c2 = INPUT( vec2( tc.x, tc.y + yof ) );\n"
"	diff += abs(c2.r - c1.r) + abs(c2.g - c1.g) + abs(c2.b - c1.b);\n"
"	c2 = INPUT( vec2( tc.x - xof, tc.y ) );\n"
"	diff += abs(c2.r - c1.r) + abs(c2.g - c1.g) + abs(c2.b - c1.b);\n"
"	c2 = INPUT( vec2( tc.x, tc.y - yof ) );\n"
"	diff += abs(c2.r - c1.r) + abs(c2.g - c1.g) + abs(c2.b - c1.b);\n"
"	diff *= PREFIX(amp);\n"
"	diff = clamp( diff, 0.0, 1.0 );\n"
"	vec4 top = vec4( vec3( 0.0 ), diff ) * c1.a;\n"
"	vec4 bottom = vec4(1.0, 1.0, 1.0, c1.a);\n"
"	return (top + (1.0 - top.a) * bottom) * c1.a;\n"
"}\n";



class DenoiseEdgeEffect : public Effect {
public:
	DenoiseEdgeEffect() : amp(3.0), iwidth(1), iheight(1) {
		register_float("amp", &amp);
	}
	std::string effect_type_id() const { return "DenoiseEdgeEffect"; }
	std::string output_fragment_shader() { return DenoiseEdgeEffect_frag; }
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
	float iwidth, iheight;
};



static const char *MyDenoiseMask_shader=
"vec4 FUNCNAME( vec2 tc ) {\n"
"	float edge = INPUT1(tc).r;\n"
"	vec4 blur = INPUT2(tc);\n"
"	vec4 org = INPUT3(tc);\n"
"	return mix( org, blur, edge );\n"
"}\n";



class MyDenoiseMask : public Effect {
public:
	MyDenoiseMask() {}
	virtual std::string effect_type_id() const { return "MyDenoiseMask"; }
	virtual unsigned num_inputs() const { return 3; }
	virtual std::string output_fragment_shader() { return MyDenoiseMask_shader; }
};



class MyDenoiseEffect : public Effect {
public:
	MyDenoiseEffect();
	std::string effect_type_id() const { return "MyDenoiseEffect"; }
	void rewrite_graph(EffectChain *graph, Node *self);
	bool set_float(const std::string &key, float value);
	std::string output_fragment_shader() { assert(false); }

private:
	BlurEffect *blur, *eblur;
	DenoiseEdgeEffect *edge;
	MyDenoiseMask *mask;
};



class GLDenoise : public GLFilter
{
public:
	GLDenoise( QString id, QString name );

	bool process( const QList<Effect*>&, Frame *src, Profile *p );
	QList<Effect*> getMovitEffects();

private:
	Parameter *amp, *eblur, *blur;
};

#endif //GLDENOISE_H
