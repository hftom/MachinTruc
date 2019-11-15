#ifndef GLMIX_H
#define GLMIX_H

#include "vfx/glfilter.h"



class GLMix : public GLFilter
{
	Q_OBJECT
public:
	GLMix( QString id, QString name );

	bool process( const QList<Effect*>&, double pts, Frame *first, Frame *second, Profile *p );
	QList<Effect*> getMovitEffects();

private:
	Parameter *mix;
};

#endif //GLMIX_H
