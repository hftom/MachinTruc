#ifndef MOVITBACKGROUND_H
#define MOVITBACKGROUND_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



class MovitBackgroundEffect : public Effect {
public:
	MovitBackgroundEffect() {}
	
	std::string effect_type_id() const { return "MovitBackgroundEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("movit_background.frag"); }
	bool needs_srgb_primaries() const { return false; }
	AlphaHandling alpha_handling() const { return INPUT_PREMULTIPLIED_ALPHA_KEEP_BLANK; }
};



class MovitBackground : public GLFilter
{
public:
	MovitBackground( QString id = "MovitBackground", QString name = "MovitBackground" );
	QList<Effect*> getMovitEffects();
};

#endif //MOVITBACKGROUND_H
