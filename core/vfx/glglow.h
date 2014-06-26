#ifndef GLGLOW_H
#define GLGLOW_H

#include "vfx/glfilter.h"



class GLGlow : public GLFilter
{
public:
    GLGlow( QString id, QString name );
    ~GLGlow();

    bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();

private:
	float radius;
	float glow;
	float highlight;
};

#endif //GLGLOW_H
