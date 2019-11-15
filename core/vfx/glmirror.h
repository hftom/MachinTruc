#ifndef GLMIRROR_H
#define GLMIRROR_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



static const char *MMirrorEffect_shader=
"vec4 FUNCNAME( vec2 tc ) {\n"
"	if (PREFIX(horizontal) > 0.0)\n"
"		tc.x = 1.0 - tc.x;\n"
"	if (PREFIX(vertical) > 0.0)\n"
"		tc.y = 1.0 - tc.y;\n"
"	return INPUT( tc );\n"
"}\n";



class MMirrorEffect : public Effect {
public:
	MMirrorEffect() : vertical(0.0), horizontal(0.0) {
		register_float("vertical", &vertical);
		register_float("horizontal", &horizontal);
	}
	virtual std::string effect_type_id() const { return "MMirrorEffect"; }
	std::string output_fragment_shader() { return MMirrorEffect_shader; }
	
private:
	float vertical, horizontal;
};



class GLMirror : public GLFilter
{
	Q_OBJECT
public:
	GLMirror( QString id, QString name );
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	QList<Effect*> getMovitEffects();
	
private:
	Parameter *horizontal, *vertical;
};

#endif //GLMIRROR_H
