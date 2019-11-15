#ifndef GLDIFFUSION_H
#define GLDIFFUSION_H

#include "vfx/glfilter.h"



class GLDiffusion : public GLFilter
{
	Q_OBJECT
public:
	GLDiffusion( QString id, QString name );
	~GLDiffusion();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();

private:
	Parameter *mixAmount, *blurRadius;
};

#endif //GLDIFFUSION_H
