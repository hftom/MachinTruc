#ifndef GLOVERLAY_H
#define GLOVERLAY_H

#include "vfx/glcomposition.h"



class GLOverlay : public GLComposition
{
public:
	GLOverlay();
	~GLOverlay();

	bool process( Effect *e, Frame *src, Frame *dst, Profile *p );
	Effect* getMovitEffect();
};

#endif //GLOVERLAY_H
