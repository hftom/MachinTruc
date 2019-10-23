#include "vfx/gllaser.h"



GLLaser::GLLaser( QString id, QString name ) : GLFilter( id, name )
{
	//iterations = addParameter( "iterations", tr("Iterations:"), Parameter::PINT, 2.0, 1.0, 4.0, false );
}



bool GLLaser::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Q_UNUSED( src );
	Effect *e = el.first();
	return e->set_float( "time", pts / MICROSECOND )
			//&& e->set_float( "iterations", getParamValue( iterations, pts ).toFloat() )
			&& e->set_float( "iwidth", src->glWidth )
			&& e->set_float( "iheight", src->glHeight );
}



QList<Effect*> GLLaser::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyLaserEffect() );
	return list;
}
