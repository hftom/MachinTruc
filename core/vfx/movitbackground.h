#ifndef MOVITBACKGROUND_H
#define MOVITBACKGROUND_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



static const char *MovitBackgroundEffect_shader=
"vec4 FUNCNAME(vec2 tc) {\n"
"	vec4 top = INPUT(tc);\n"
"	return top + (1.0 - top.a) * vec4(0.0, 0.0, 0.0, 1.0);\n"
"}\n";



class MovitBackgroundEffect : public Effect {
public:
	MovitBackgroundEffect() {}
	
	std::string effect_type_id() const { return "MovitBackgroundEffect"; }
	std::string output_fragment_shader() { return MovitBackgroundEffect_shader; }
};



class MovitBackground : public GLFilter
{
public:
	MovitBackground( QString id = "MovitBackground", QString name = "MovitBackground" );
	QList<Effect*> getMovitEffects();
};

#endif //MOVITBACKGROUND_H
