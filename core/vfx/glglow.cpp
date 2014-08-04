#include <movit/glow_effect.h>
#include "vfx/glglow.h"



GLGlow::GLGlow( QString id, QString name ) : GLFilter( id, name )
{
	radius = addParameter( tr("Radius:"), Parameter::PDOUBLE, 20.0, 0.0, 100.0, true );
	glow = addParameter( tr("Glow:"), Parameter::PDOUBLE, 1.0, 0.0, 10.0, true );
	highlight = addParameter( tr("Highlight:"), Parameter::PDOUBLE, 0.2, 0.0, 1.0, true );
}



GLGlow::~GLGlow()
{
}



bool GLGlow::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	double pts = src->pts();
	Effect *e = el[0];
	return e->set_float( "radius", getParamValue( radius, pts ) )
		&& e->set_float( "blurred_mix_amount", getParamValue( glow, pts ) )
		&& e->set_float( "highlight_cutoff", getParamValue( highlight, pts ) );
}



QList<Effect*> GLGlow::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new GlowEffect() );
	return list;
}
