#ifndef GLRESIZE_H
#define GLRESIZE_H

#include "vfx/glfilter.h"



class GLResize : public GLFilter
{
public:
    GLResize( QString id = "ResizeAuto", QString name = "ResizeAuto" );
    ~GLResize();

    bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();
	
	float width, height;
	
private:
	float percent;
};

#endif //GLRESIZE_H
