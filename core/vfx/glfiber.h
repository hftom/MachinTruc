#ifndef GLFIBER_H
#define GLFIBER_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



static const char *MyFiberEffect_shader=
"vec4 FUNCNAME( vec2 tc ) {\n"
"	tc -= 0.5;\n"
"	float vertColor = 0.0;\n"
"	for( float i = 0.0; i < PREFIX(iterations); i++ ) {\n"
"		float t = PREFIX(time) * 0.5;\n"
"		tc.y += sin( t + tc.x * 6.0 ) * 0.45;\n"
"		tc.x += sin( -t + tc.y * 3.0 ) * 0.25;\n"
"		float value = atan( tc.y * 2.5 ) + sin( tc.x * 10.0 );\n"
"		float stripColor = 1.0 / sqrt( abs( value ) );\n"
"		vertColor += stripColor / 14.0 / PREFIX(iterations);\n"
"	}\n"
"	return clamp( vec4( vec3( vertColor * 0.5, vertColor / 10.0, vertColor / 15.0 ), 1.0 ), vec4(0.0), vec4(1.0) );\n"
"}\n";



class MyFiberEffect : public Effect {
public:
	MyFiberEffect() : time(0), iterations(2) {
		register_float("time", &time);
		register_float("iterations", &iterations);
	}
	std::string effect_type_id() const { return "MyFiberEffect"; }
	std::string output_fragment_shader() { return MyFiberEffect_shader; }
	
private:
	float time, iterations;
};



class GLFiber : public GLFilter
{
public:
	GLFiber( QString id, QString name );

	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *iterations;
};

#endif // GLFIBER_H
