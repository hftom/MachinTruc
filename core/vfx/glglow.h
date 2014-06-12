#ifndef GLGLOW_H
#define GLGLOW_H

#include "vfx/glfilter.h"



class GLGlow : public GLFilter
{
public:
    GLGlow( QString id, QString name );
    ~GLGlow();

    bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();

private:
	float radius;
	float glow;
	float highlight;
};

#endif //GLGLOW_H
