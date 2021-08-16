#ifndef GLNOISE_H
#define GLNOISE_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



class MyNoiseEffect : public Effect {
public:
	MyNoiseEffect() : time(0) {
		register_float("time", &time);
	}
	std::string effect_type_id() const { return "MyNoiseEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("noise.frag"); }
	
private:
	float time;
};



class GLNoise : public GLFilter
{
	Q_OBJECT
public:
	GLNoise( QString id, QString name );

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
};

#endif // GLNOISE_H
