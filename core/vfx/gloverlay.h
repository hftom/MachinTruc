#ifndef GLOVERLAY_H
#define GLOVERLAY_H

#include "vfx/glfilter.h"



class GLOverlay : public GLFilter
{
public:
	GLOverlay( QString id = "OverlayAuto", QString name = "OverlayAuto" );

	QList<Effect*> getMovitEffects();
};

#endif //GLOVERLAY_H
