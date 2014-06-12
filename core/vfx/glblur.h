#ifndef GLBLUR_H
#define GLBLUR_H

#include "vfx/glfilter.h"



class GLBlur : public GLFilter
{
public:
    GLBlur( QString id, QString name );
    ~GLBlur();

    bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();

private:
	float radius;
};

#endif //GLBLUR_H
