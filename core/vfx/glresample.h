#ifndef GLRESAMPLE_H
#define GLRESAMPLE_H

#include "vfx/glresize.h"



class GLResample : public GLResize
{
public:
	GLResample( QString id = "ResampleAuto", QString name = "ResampleAuto" );

	virtual QList<Effect*> getMovitEffects();
};

#endif //GLRESAMPLE_H
