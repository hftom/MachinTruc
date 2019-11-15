#ifndef GLNOISE_H
#define GLNOISE_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



static const char *MyNoiseEffect_shader=
"vec4 FUNCNAME( vec2 tc ) {\n"
"	float col = fract( sin( dot((tc + 0.007 * fract(PREFIX(time))) ,vec2(12.9898,78.233)) ) * 43758.5453 );\n"
"	return vec4( col, col, col, 1.0 );\n"
"}\n";



class MyNoiseEffect : public Effect {
public:
	MyNoiseEffect() : time(0) {
		register_float("time", &time);
	}
	std::string effect_type_id() const { return "MyNoiseEffect"; }
	std::string output_fragment_shader() { return MyNoiseEffect_shader; }
	
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
