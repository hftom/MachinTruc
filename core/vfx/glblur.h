#ifndef GLBLUR_H
#define GLBLUR_H

#include "vfx/glmask.h"

#include <movit/blur_effect.h>



class GLBlur : public GLMask
{
public:
	GLBlur( QString id, QString name );

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	QString getDescriptor( double pts, Frame *src, Profile *p  );

	QList<Effect*> getMovitEffects();

private:
	Parameter *amount;
};

#endif //GLBLUR_H
