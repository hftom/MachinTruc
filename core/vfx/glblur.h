#ifndef GLBLUR_H
#define GLBLUR_H

#include "vfx/glmask.h"



class GLBlur : public GLMask
{
	Q_OBJECT
public:
	GLBlur( QString id, QString name );
	~GLBlur();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	QString getDescriptor( double pts, Frame *src, Profile *p  );

	QList<Effect*> getMovitEffects();

private:
	Parameter *amount;
};

#endif //GLBLUR_H
