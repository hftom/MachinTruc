#ifndef GLBACKGROUNDCOLOR_H
#define GLBACKGROUNDCOLOR_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "glfilter.h"



static const char *MyBackgroundColorEffect_shader=
"vec4 FUNCNAME(vec2 tc) {\n"
"	vec4 top = INPUT(tc);\n"
"	return top + (1.0 - top.a) * PREFIX(color);\n"
"}\n";



class MyBackgroundColorEffect : public Effect {
public:
	MyBackgroundColorEffect();
	virtual std::string effect_type_id() const { return "MyBackgroundColorEffect"; }
	std::string output_fragment_shader() { return MyBackgroundColorEffect_shader; }

private:
	RGBATuple color;
};



class GLBackgroundColor : public GLFilter
{
public:
	GLBackgroundColor( QString id, QString name );
	
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *color;
};

#endif // GLBACKGROUNDCOLOR_H
