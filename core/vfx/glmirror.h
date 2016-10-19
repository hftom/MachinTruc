#ifndef GLMIRROR_H
#define GLMIRROR_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



static const char *MMirrorEffect_shader=
"vec4 FUNCNAME( vec2 tc ) {\n"
"	tc.x = 1.0 - tc.x;\n"
"	return INPUT( tc );\n"
"}\n";



class MMirrorEffect : public Effect {
public:
	virtual std::string effect_type_id() const { return "MMirrorEffect"; }
	std::string output_fragment_shader() { return MMirrorEffect_shader; }
};



class GLMirror : public GLFilter
{
public:
	GLMirror( QString id, QString name );
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	QList<Effect*> getMovitEffects();
};

#endif //GLMIRROR_H
