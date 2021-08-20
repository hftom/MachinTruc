#ifndef GLBACKGROUNDCOLOR_H
#define GLBACKGROUNDCOLOR_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "glfilter.h"



class MyBackgroundColorEffect : public Effect {
public:
	MyBackgroundColorEffect();
	virtual std::string effect_type_id() const { return "MyBackgroundColorEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("background_color.frag"); }

private:
	RGBATuple color;
};



class GLBackgroundColor : public GLFilter
{
	Q_OBJECT
public:
	GLBackgroundColor( QString id, QString name );
	
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *color;
};

#endif // GLBACKGROUNDCOLOR_H
