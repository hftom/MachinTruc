#ifndef GLDIFFUSION_H
#define GLDIFFUSION_H

#include "vfx/glfilter.h"



class GLDiffusion : public GLFilter
{
public:
    GLDiffusion( QString id, QString name );
    ~GLDiffusion();

    bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();

private:
	float mixAmount;
	float blurRadius;
};

#endif //GLDIFFUSION_H
