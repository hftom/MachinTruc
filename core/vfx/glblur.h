#ifndef GLBLUR_H
#define GLBLUR_H

#include "vfx/glfilter.h"



class GLBlur : public GLFilter
{
public:
	GLBlur( QString id, QString name );
	~GLBlur();

	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();

private:
	Parameter *amount;
};

#endif //GLBLUR_H
