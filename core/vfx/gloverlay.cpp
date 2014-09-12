#include <movit/overlay_effect.h>
#include "vfx/gloverlay.h"



GLOverlay::GLOverlay( QString id, QString name ) : GLFilter( id, name )
{
}



QList<Effect*> GLOverlay::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new OverlayEffect() );
	return list;
}
