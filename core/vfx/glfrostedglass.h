#ifndef GLFROSTEDGLASS_H
#define GLFROSTEDGLASS_H

#include <movit/blur_effect.h>
#include <movit/mix_effect.h>
#include <movit/effect_util.h>
#include <movit/util.h>

#include "glfilter.h"



class MySlidingWindow : public Effect {
public:
	MySlidingWindow() : position(0) {
		register_float( "position", &position );
	}
	
	virtual std::string effect_type_id() const { return "MySlidingWindow"; }
	std::string output_fragment_shader() { return GLFilter::getShader("sliding_window.frag"); }
	virtual bool needs_srgb_primaries() const { return false; }
	virtual unsigned num_inputs() const { return 2; }

private:
	float position;
};



class MyFrostedGlassEffect : public Effect {
public:
	MyFrostedGlassEffect();
	virtual std::string effect_type_id() const { return "MyFrostedGlassEffect"; }
	virtual unsigned num_inputs() const { return 2; }
	virtual void rewrite_graph( EffectChain *graph, Node *self );
	virtual bool set_float( const std::string &key, float value );
	virtual std::string output_fragment_shader() { return GLFilter::getShader("mix_window.frag"); }

private:
	BlurEffect *blur1, *blur2;
	MySlidingWindow *cover1, *cover2;
	float strength_first, strength_second, position;
};



class GLFrostedGlass : public GLFilter
{
	Q_OBJECT
public:
	GLFrostedGlass( QString id, QString name );

	bool process( const QList<Effect*>&, double pts, Frame *first, Frame *second, Profile *p );
	QList<Effect*> getMovitEffects();

private:
	Parameter *position, *mixAmount;
};

#endif //GLFROSTEDGLASS_H
