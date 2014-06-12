#ifndef GLBLURMASK_H
#define GLBLURMASK_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"

#include <movit/blur_effect.h>



static const char *mask_shader=
"uniform sampler2D PREFIX(mask);\n"
"\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	//float m = texture2D(PREFIX(mask), tc).x;\n"
"	float f=1.0; float m = texture2D(PREFIX(mask), tc/f + (f - 1.0)/(2.0 * f) ).x;\n"
"	return mix( INPUT1(tc), INPUT2(tc), m );\n"
"}\n";



//class BlurEffect;
//class Node;



//#include <string>


class MaskEffect : public Effect {
public:
	MaskEffect() {
		mask_texture_num = 1;
	}
	std::string effect_type_id() const { return "MaskEffect"; }
	std::string output_fragment_shader() {
		return mask_shader;
	}
	void set_gl_state(GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num) {
		glActiveTexture(GL_TEXTURE0 + *sampler_num);
		check_error();
		glBindTexture(GL_TEXTURE_2D, mask_texture_num);
		check_error();
		// Bind it to a sampler.
		set_uniform_int(glsl_program_num, prefix, "mask", *sampler_num);
		++*sampler_num;
	}

	bool needs_srgb_primaries() const { return false; }
	unsigned num_inputs() const { return 2; }

private:
	GLuint mask_texture_num;
};



class BlurEffectMask : public Effect {
public:
	BlurEffectMask();
	std::string effect_type_id() const { return "BlurEffectMask"; }

	bool needs_srgb_primaries() const { return false; }

	void rewrite_graph(EffectChain *graph, Node *self);
	bool set_float(const std::string &key, float value);

	std::string output_fragment_shader() {
		assert(false);
	}
	/*virtual void set_gl_state(GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num) {
		assert(false);
	}*/

private:
	BlurEffect *blur;
	MaskEffect *maskfx;
};



class GLBlurmask : public GLFilter
{
    Q_OBJECT
public:
    GLBlurmask( QString id, QString name );
    ~GLBlurmask();

    bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();

public slots:
    //QStringList getProperties();
    //void setProperty( QStringList prop );

private:
	float radius;
	GLuint mask_texture_num;
};

#endif //GLBLURMASK_H
