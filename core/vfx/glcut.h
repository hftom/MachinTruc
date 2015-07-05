#ifndef GLCUT_H
#define GLCUT_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



static const char *MyCutEffect_frag=
"uniform sampler2D PREFIX(mask);\n"
"\n"
"vec4 FUNCNAME( vec2 tc ) {\n"
"	vec4 col = INPUT( tc );\n"
"	return col - ( col * (1.0 - texture2D( PREFIX(mask), tc ).x) * PREFIX(opacity) );\n"
"}\n";



class MyCutEffect : public Effect {
public:
	MyCutEffect() : opacity( 1.0f ) {
		register_float("opacity", &opacity);
	}
	std::string effect_type_id() const { return "MyCutEffect"; }
	std::string output_fragment_shader() { return MyCutEffect_frag; }
	void set_gl_state(GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num) {
		Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		glActiveTexture(GL_TEXTURE0 + *sampler_num);
		check_error();
		glBindTexture(GL_TEXTURE_2D, 3);
		check_error();
		// Bind it to a sampler.
		set_uniform_int(glsl_program_num, prefix, "mask", *sampler_num);
		++*sampler_num;
	}
	
private:
	float opacity;
};



class GLCut : public GLFilter
{
public:
	GLCut( QString id, QString name );
	~GLCut();
	
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *opacity;
};

#endif // GLCUT_H
