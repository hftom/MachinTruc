#include "vfx/glwater.h"



GLWater::GLWater( QString id, QString name ) : GLFilter( id, name )
{
}



GLWater::~GLWater()
{
}



bool GLWater::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p )
	return el.at(0)->set_float( "time", src->pts() / MICROSECOND )
		&& el.at(0)->set_float( "inwidth", src->glWidth )
		&& el.at(0)->set_float( "inheight", src->glHeight );
}



QList<Effect*> GLWater::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new WaterEffect() );
	return list;
}