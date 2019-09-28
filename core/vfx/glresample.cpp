#include <movit/resample_effect.h>
#include "vfx/glresample.h"



GLResample::GLResample( QString id, QString name ) : GLResize( id, name )
{
}



QList<Effect*> GLResample::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new ResampleEffect() );
	return list;
}
