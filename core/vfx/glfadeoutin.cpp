#include "glfadeoutin.h"
#include "engine/util.h"



GLFadeOutIn::GLFadeOutIn( QString id, QString name ) : GLFilter( id, name )
{
	pause = addParameter( "pause", tr("Black length:"), Parameter::PDOUBLE, 0.0, 0.0, 0.5, false );
}



bool GLFadeOutIn::process( const QList<Effect*> &el, double pts, Frame *first, Frame *second, Profile *p )
{
	Q_UNUSED( first );
	Q_UNUSED( second );
	Q_UNUSED( p );
	double npts = getNormalizedTime( pts );
	double lpause = getParamValue( pause ).toDouble();
	double opacity = 0.0;
	if ( npts <= 0.5 - lpause / 2.0 ) {
		opacity = cosineInterpolate( 1.0, 0.0, (npts * 2.0) / (1.0 - lpause) );
	}
	else if ( npts >= 0.5 + lpause / 2.0 ) {
		opacity = cosineInterpolate( 0.0, 1.0, (npts - 0.5 - lpause / 2.0) * 2.0 / (1.0 - lpause) );
	}
	
	Effect *e = el[0];
	return e->set_float( "show_second", npts > 0.5 ? 1 : 0 )
		&& e->set_float( "opacity", opacity );
}



QList<Effect*> GLFadeOutIn::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new MyFadeOutInEffect() );
	return list;
}
