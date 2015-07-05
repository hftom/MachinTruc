#include <movit/glow_effect.h>
#include "vfx/glglow.h"



GLGlow::GLGlow( QString id, QString name ) : GLFilter( id, name )
{
	radius = addParameter( "radius", tr("Radius:"), Parameter::PDOUBLE, 20.0, 0.0, 100.0, true );
	glow = addParameter( "glow", tr("Glow:"), Parameter::PDOUBLE, 1.0, 0.0, 10.0, true );
	highlight = addParameter( "highlight", tr("Highlight:"), Parameter::PDOUBLE, 0.2, 0.0, 1.0, true );
}



GLGlow::~GLGlow()
{
}



bool GLGlow::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	Q_UNUSED( src );
	Effect *e = el[0];
	return e->set_float( "radius", getParamValue( radius, pts ).toFloat() )
		&& e->set_float( "blurred_mix_amount", getParamValue( glow, pts ).toFloat() )
		&& e->set_float( "highlight_cutoff", getParamValue( highlight, pts ).toFloat() );
}



QList<Effect*> GLGlow::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new GlowEffect() );
	return list;
}
