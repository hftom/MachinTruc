#ifndef GLVIGNETTE_H
#define GLVIGNETTE_H

#include "vfx/glfilter.h"



class GLVignette : public GLFilter
{
public:
    GLVignette( QString id, QString name );
    ~GLVignette();

    bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();

private:
    float inner_radius;
	float radius;
	float center[2];
};

#endif //GLVIGNETTE_H
