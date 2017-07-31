#ifndef GLBLUR_H
#define GLBLUR_H

#include "vfx/glfilter.h"

#include <movit/blur_effect.h>



class GLBlur : public GLFilter
{
public:
	GLBlur( QString id, QString name );
	~GLBlur();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();

private:
	Parameter *amount;
};

#endif //GLBLUR_H
