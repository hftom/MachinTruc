#ifndef GLVIGNETTE_H
#define GLVIGNETTE_H

#include "vfx/glfilter.h"



class GLVignette : public GLFilter
{
public:
    GLVignette( QString id, QString name );
    ~GLVignette();

    bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();

private:
    float inner_radius;
	float radius;
	float center[2];
};

#endif //GLVIGNETTE_H
