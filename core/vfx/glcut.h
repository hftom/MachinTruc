#ifndef GLCUT_H
#define GLCUT_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



static const char *cutshader=
"uniform sampler2D PREFIX(mask);\n"
"\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	return INPUT(tc) * texture2D(PREFIX(mask), tc).x;\n"
"}\n";



class MyCutEffect : public Effect {
public:
	MyCutEffect() {}
	std::string effect_type_id() const { return "MyCutEffect"; }
	std::string output_fragment_shader() { return cutshader; }
	void set_gl_state(GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num) {
		glActiveTexture(GL_TEXTURE0 + *sampler_num);
		check_error();
		glBindTexture(GL_TEXTURE_2D, 3);
		check_error();
		// Bind it to a sampler.
		set_uniform_int(glsl_program_num, prefix, "mask", *sampler_num);
		++*sampler_num;
	}
};



class GLCut : public GLFilter
{
public:
    GLCut( QString id, QString name );
    ~GLCut();

	Effect* getMovitEffect();
};

#endif // GLCUT_H
