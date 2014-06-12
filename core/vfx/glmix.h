#ifndef GLMIX_H
#define GLMIX_H

#include "vfx/glcomposition.h"



class GLMix : public GLComposition
{
public:
    GLMix();
    ~GLMix();

	bool process( Effect *e, Frame *src, Frame *dst, Profile *p );
	Effect* getMovitEffect();

private:
	float strength_first;
	float strength_second;

};

#endif //GLMIX_H
