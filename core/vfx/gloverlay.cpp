#include <movit/overlay_effect.h>
#include "vfx/gloverlay.h"



GLOverlay::GLOverlay() : GLComposition()
{
	compositionName = "OverlayEffect";
	invert = 0;
}



GLOverlay::~GLOverlay()
{
}



bool GLOverlay::process( Effect *e, Frame *src, Frame *dst, Profile *p )
{
	Q_UNUSED( e );
	Q_UNUSED( src );
	Q_UNUSED( dst );
	Q_UNUSED( p );
	return true;
}



Effect* GLOverlay::getMovitEffect()
{
	return new OverlayEffect();
}
