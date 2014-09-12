#ifndef GLMIX_H
#define GLMIX_H

#include "vfx/glfilter.h"



class GLMix : public GLFilter
{
public:
	GLMix( QString id, QString name );

	bool process( const QList<Effect*>&, Frame *src, Frame *dst, Profile *p );
	QList<Effect*> getMovitEffects();

private:
	Parameter *mix;
};

#endif //GLMIX_H
