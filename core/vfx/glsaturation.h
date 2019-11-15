#ifndef GLSATURATION_H
#define GLSATURATION_H

#include <movit/effect_util.h>

#include "vfx/glmask.h"

#include <movit/saturation_effect.h>



class GLSaturation : public GLMask
{
	Q_OBJECT
public:
	GLSaturation( QString id, QString name );
	~GLSaturation();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	QString getDescriptor( double pts, Frame *src, Profile *p  );

	QList<Effect*> getMovitEffects();

private:
	Parameter *saturation;
	GLuint mask_texture_num;
};

#endif //GLSATURATION_H
