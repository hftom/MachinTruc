#ifndef GLCONTRAST_H
#define GLCONTRAST_H

#include <movit/effect_util.h>

#include "vfx/glmask.h"



class MContrastEffect : public Effect {
public:
	MContrastEffect() : contrast(0.0), brightness(0.0) {
		register_float("contrast", &contrast);
		register_float("brightness", &brightness);
	}
	virtual std::string effect_type_id() const { return "MContrastEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("contrast.frag"); }

private:
	float contrast, brightness;
};



class GLContrast : public GLMask
{
	Q_OBJECT
public:
	GLContrast( QString id, QString name );
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	QString getDescriptor( double pts, Frame *src, Profile *p  );
	QList<Effect*> getMovitEffects();

private:
	Parameter *contrast, *brightness;
};

#endif //GLCONTRAST_H
