#include <movit/blur_effect.h>

#include "vfx/gledge.h"



GLEdge::GLEdge( QString id, QString name ) : GLFilter( id, name )
{
	amp = addParameter( "amp", tr("Amount:"), Parameter::PDOUBLE, 5.0, 0.0, 20.0, true );
	depth = addParameter( "depth", tr("Depth:"), Parameter::PDOUBLE, 1.0, 0.0, 1.0, true );
	opacity = addParameter( "opacity", tr("Opacity:"), Parameter::PDOUBLE, 1.0, 0.0, 1.0, true );
	blur = addParameter( "blur", tr("Blur radius:"), Parameter::PDOUBLE, 1.0, 0.0, 4.0, false );
}



GLEdge::~GLEdge()
{
}



bool GLEdge::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	Effect *e = el[1];
	return el.at(0)->set_float( "radius", getParamValue( blur ).toFloat() )
		&& e->set_float( "amp", getParamValue( amp, pts ).toFloat() )
		&& e->set_float( "depth", 1.0f - getParamValue( depth, pts ).toFloat() )
		&& e->set_float( "opacity", getParamValue( opacity, pts ).toFloat() );
}



QList<Effect*> GLEdge::getMovitEffects()
{
	QList<Effect*> list;
	BlurEffect* blur = new BlurEffect();
	bool ok = blur->set_int( "num_taps", 6 );
	Q_UNUSED( ok );
	list.append( blur );
	list.append( new EdgeEffect() );
	return list;
}
