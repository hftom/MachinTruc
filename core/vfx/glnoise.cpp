#include "vfx/glnoise.h"



GLNoise::GLNoise( QString id, QString name ) : GLFilter( id, name )
{
}



bool GLNoise::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Q_UNUSED( src );
	return el.first()->set_float( "time", pts / MICROSECOND );
}



QList<Effect*> GLNoise::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyNoiseEffect() );
	return list;
}
