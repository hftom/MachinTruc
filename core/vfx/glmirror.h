#ifndef GLMIRROR_H
#define GLMIRROR_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



class MyMirrorEffect : public Effect {
public:
	MyMirrorEffect() : vertical(0.0), horizontal(1.0) {
		register_float("vertical", &vertical);
		register_float("horizontal", &horizontal);
	}
	virtual std::string effect_type_id() const { return "MyMirrorEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("mirror.frag"); }
	bool needs_linear_light() const { return false; }
	bool needs_srgb_primaries() const { return false; }
	AlphaHandling alpha_handling() const { return DONT_CARE_ALPHA_TYPE; }
	
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
