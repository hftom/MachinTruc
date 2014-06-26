#ifndef GLRESIZE_H
#define GLRESIZE_H

#include "vfx/glfilter.h"



class GLResize : public GLFilter
{
public:
    GLResize( QString id = "ResizeAuto", QString name = "ResizeAuto" );
    ~GLResize();

    bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
	float width, height;
	
private:
	float percent;
};

#endif //GLRESIZE_H
