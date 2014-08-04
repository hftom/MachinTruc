#ifndef GLEDGE_H
#define GLEDGE_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



static const char *EdgeEffect_frag=
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
"	if ( diff < PREFIX(depth) )\n"
"		diff = 0.0;\n"
"	diff = clamp( diff, 0.0, 1.0 );\n"
"	vec4 top = vec4( vec3( 0.0 ), diff ) * c1.a;\n"
"	vec4 bottom = mix( c1, vec4(1.0, 1.0, 1.0, c1.a), PREFIX(opacity));\n"
"	return (top + (1.0 - top.a) * bottom) * c1.a;\n"
"}\n";



class EdgeEffect : public Effect {
public:
	EdgeEffect() : amp(3.0), depth(0.3), opacity(1.0) {
		register_float("amp", &amp);
		register_float("depth", &depth);
		register_float("opacity", &opacity);
	}
	std::string effect_type_id() const { return "EdgeEffect"; }
	std::string output_fragment_shader() { return EdgeEffect_frag; }
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
public:
    GLEdge( QString id, QString name );
    ~GLEdge();

    bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *amp, *depth, *opacity, *blur;
};

#endif // GLEDGE_H
