#ifndef FLIPEFFEXT_H
#define FLIPEFFEXT_H

#include <movit/effect.h>



static const char *MyFlipEffect_shader=
"vec4 FUNCNAME(vec2 tc) {\n"
"	tc.y = 1.0 - tc.y;\n"
"	return INPUT(tc);\n"
"}\n";



class MyFlipEffect : public Effect {
public:
	MyFlipEffect() {}
	virtual std::string effect_type_id() const { return "MyFlipEffect"; }
	std::string output_fragment_shader() { return MyFlipEffect_shader; }

	virtual bool needs_linear_light() const { return false; }
	virtual bool needs_srgb_primaries() const { return false; }
	virtual AlphaHandling alpha_handling() const { return DONT_CARE_ALPHA_TYPE; }
};

#endif // FLIPEFFEXT_H



