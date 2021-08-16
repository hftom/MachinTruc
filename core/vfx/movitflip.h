#ifndef FLIPEFFEXT_H
#define FLIPEFFEXT_H

#include <movit/effect.h>



class MyFlipEffect : public Effect {
public:
	MyFlipEffect() {}
	virtual std::string effect_type_id() const { return "MyFlipEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("movit_flip.frag"); }

	virtual bool needs_linear_light() const { return false; }
	virtual bool needs_srgb_primaries() const { return false; }
	virtual AlphaHandling alpha_handling() const { return DONT_CARE_ALPHA_TYPE; }
};

#endif // FLIPEFFEXT_H



