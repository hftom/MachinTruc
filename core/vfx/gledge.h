#ifndef GLEDGE_H
#define GLEDGE_H

#include "vfx/glfilter.h"



static const char *filter_edge_frag=
"vec4 FUNCNAME( vec2 tc ) {\n"
"	float xof = 1.0 / PREFIX(width);\n"
"	float yof = 1.0 / PREFIX(height);\n"
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
"	return (c1 * (1.0 - PREFIX(opacity))) + (clamp(vec4( vec3( (1.0 - diff) * c1.a ), c1.a ), 0.0, 1.0) * PREFIX(opacity));\n"
"}\n";


class EdgeEffect : public Effect {
public:
	EdgeEffect() : amp(3.0), depth(0.3), opacity(1.0), width(1280), height(720) {
		register_float("amp", &amp);
		register_float("depth", &depth);
		register_float("opacity", &opacity);
		register_float("width", &width);
		register_float("height", &height);
	}
	std::string effect_type_id() const { return "EdgeEffect"; }
	std::string output_fragment_shader() { return filter_edge_frag; }
	bool needs_texture_bounce() const { return true; }
	
private:
	float amp;
	float depth;
	float opacity;
	float width, height;
};



class GLEdge : public GLFilter
{
public:
    GLEdge( QString id, QString name );
    ~GLEdge();

    bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();
	
private:
	float amp;
	float depth;
	float opacity;
};

#endif // GLEDGE_H
