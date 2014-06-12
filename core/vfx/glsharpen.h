#ifndef GLSHARPEN_H
#define GLSHARPEN_H

#include "vfx/glfilter.h"



class GLSharpen : public GLFilter
{
public:
    GLSharpen( QString id, QString name );
    ~GLSharpen();

    bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();

private:
	float amount;
};

#endif //GLSHARPEN_H
