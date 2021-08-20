#ifndef GLKALEIDOSCOPE_H
#define GLKALEIDOSCOPE_H

#include <movit/effect.h>
#include <movit/effect_util.h>

#include "vfx/glfilter.h"



class KaleidoscopeEffect : public Effect {
public:
	KaleidoscopeEffect() : time(0.0), size(7.0)
	{
		register_float( "time", &time );
		register_float( "size", &size );
	}

	virtual std::string effect_type_id() const { return "KaleidoscopeEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("kaleidoscope.frag"); }

private:
	float time, size;
};



class GLKaleidoscope : public GLFilter
{
	Q_OBJECT
public:
	GLKaleidoscope( QString id, QString name );
	~GLKaleidoscope();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *size;
};

#endif //GLKALEIDOSCOPE_H
