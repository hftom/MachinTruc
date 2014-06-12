#ifndef GLSATURATION_H
#define GLSATURATION_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"

#include <movit/saturation_effect.h>



static const char *smask_shader=
"uniform sampler2D PREFIX(mask);\n"
"\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	float m = texture2D(PREFIX(mask), tc).x;\n"
"	//float f=1.0; float m = texture2D(PREFIX(mask), tc/f + (f - 1.0)/(2.0 * f) ).x;\n"
"	return mix( INPUT1(tc), INPUT2(tc), m );\n"
"}\n";



class SMaskEffect : public Effect {
public:
	SMaskEffect() {
		mask_texture_num = 3;
	}
	virtual std::string effect_type_id() const { return "MaskEffect"; }
	std::string output_fragment_shader() { return smask_shader; }
	virtual void set_gl_state(GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num) {
		glActiveTexture(GL_TEXTURE0 + *sampler_num);
		check_error();
		glBindTexture(GL_TEXTURE_2D, mask_texture_num);
		check_error();
		// Bind it to a sampler.
		set_uniform_int(glsl_program_num, prefix, "mask", *sampler_num);
		++*sampler_num;
	}

	//virtual bool needs_srgb_primaries() const { return false; }
	virtual unsigned num_inputs() const { return 2; }

private:
	GLuint mask_texture_num;
};



class MGLSaturation : public Effect {
public:
	MGLSaturation();
	virtual std::string effect_type_id() const { return "SaturationEffectMask"; }

	//virtual bool needs_srgb_primaries() const { return false; }

	virtual void rewrite_graph(EffectChain *graph, Node *self);
	virtual bool set_float(const std::string &key, float value);

	virtual std::string output_fragment_shader() {
		assert(false);
	}
	/*virtual void set_gl_state(GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num) {
		assert(false);
	}*/

private:
	SaturationEffect *sat;
	SMaskEffect *maskfx;
};



class GLSaturation : public GLFilter
{
public:
    GLSaturation( QString id, QString name );
    ~GLSaturation();

    bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();

private:
	float saturation;
	GLuint mask_texture_num;
};

#endif //GLSATURATION_H

