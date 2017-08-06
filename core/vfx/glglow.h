#ifndef GLGLOW_H
#define GLGLOW_H

#include "vfx/glmask.h"



class GLGlow : public GLMask
{
public:
	GLGlow( QString id, QString name );

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	QString getDescriptor( double pts, Frame *src, Profile *p  );

	QList<Effect*> getMovitEffects();

private:
	Parameter *radius, *glow, *highlight;
};

#endif //GLGLOW_H
