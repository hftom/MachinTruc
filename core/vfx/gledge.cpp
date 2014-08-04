#include <movit/blur_effect.h>

#include "vfx/gledge.h"



GLEdge::GLEdge( QString id, QString name ) : GLFilter( id, name )
{
	amp = addParameter( tr("Amount:"), Parameter::PDOUBLE, 5.0, 0.0, 20.0, true );
	depth = addParameter( tr("Depth:"), Parameter::PDOUBLE, 1.0, 0.0, 1.0, true );
	opacity = addParameter( tr("Opacity:"), Parameter::PDOUBLE, 1.0, 0.0, 1.0, true );
	blur = addParameter( tr("Blur radius:"), Parameter::PDOUBLE, 1.0, 0.0, 4.0, false );
}



GLEdge::~GLEdge()
{
}



bool GLEdge::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	double pts = src->pts();
	Effect *e = el[1];
	return el.at(0)->set_float( "radius", getParamValue( blur, 0 ) )
		&& e->set_float( "amp", getParamValue( amp, pts ) )
		&& e->set_float( "depth", 1.0f - getParamValue( depth, pts ) )
		&& e->set_float( "opacity", getParamValue( opacity, pts ) );
}



QList<Effect*> GLEdge::getMovitEffects()
{
	QList<Effect*> list;
	BlurEffect* blur = new BlurEffect();
	blur->set_int( "num_taps", 6 );
	list.append( blur );
	list.append( new EdgeEffect() );
	return list;
}
