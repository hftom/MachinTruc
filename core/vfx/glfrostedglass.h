#ifndef GLFROSTEDGLASS_H
#define GLFROSTEDGLASS_H

#include <movit/blur_effect.h>
#include <movit/mix_effect.h>
#include <movit/effect_util.h>
#include <movit/util.h>

#include "glfilter.h"



static const char *MySlidingWindow_shader=
"vec4 FUNCNAME( vec2 tc ) {\n"
"	if ( tc.x <= PREFIX(position) || tc.x >= 1.0 - PREFIX(position) )\n"
"		return INPUT2( tc );\n"
"	return INPUT1( tc );\n"
"}\n";



class MySlidingWindow : public Effect {
public:
	MySlidingWindow() : position(0) {
		register_float( "position", &position );
	}
	
	virtual std::string effect_type_id() const { return "MySlidingWindow"; }
	std::string output_fragment_shader() { return MySlidingWindow_shader; }
	virtual bool needs_srgb_primaries() const { return false; }
	virtual unsigned num_inputs() const { return 2; }

private:
	float position;
};



static const char *MyMixWindow_shader=
"vec4 FUNCNAME( vec2 tc ) {\n"
"	if ( tc.x > PREFIX(position) && tc.x < 1.0 - PREFIX(position) ) {\n"
"		if ( PREFIX(strength_first) > 0.5 )\n"
"			return INPUT1(tc);\n"
"		return INPUT2(tc);\n"
"	}\n"
"	vec4 first = INPUT1(tc);\n"
"	vec4 second = INPUT2(tc);\n"
"	vec4 result = vec4(PREFIX(strength_first)) * first + vec4(PREFIX(strength_second)) * second;\n"
"	result.a = clamp(result.a, 0.0, 1.0);\n"
"	return result;\n"
"}\n";



class MyFrostedGlassEffect : public Effect {
public:
	MyFrostedGlassEffect();
	virtual std::string effect_type_id() const { return "MyFrostedGlassEffect"; }
	virtual unsigned num_inputs() const { return 2; }
	virtual void rewrite_graph( EffectChain *graph, Node *self );
	virtual bool set_float( const std::string &key, float value );
	virtual std::string output_fragment_shader() { return MyMixWindow_shader; }

private:
	BlurEffect *blur1, *blur2;
	MySlidingWindow *cover1, *cover2;
	float strength_first, strength_second, position;
};



class GLFrostedGlass : public GLFilter
{
public:
	GLFrostedGlass( QString id, QString name );

	bool process( const QList<Effect*>&, Frame *src, Frame *dst, Profile *p );
	QList<Effect*> getMovitEffects();

private:
	Parameter *position, *mixAmount;
};

#endif //GLFROSTEDGLASS_H
