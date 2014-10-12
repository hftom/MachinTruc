#include "movitbackground.h"



MovitBackground::MovitBackground( QString id, QString name ) : GLFilter( id, name )
{
}



QList<Effect*> MovitBackground::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MovitBackgroundEffect() );

	return list;
}
