#include <movit/glow_effect.h>
#include "vfx/glglow.h"



GLGlow::GLGlow( QString id, QString name ) : GLMask( id, name )
{
	radius = addParameter( "radius", tr("Radius:"), Parameter::PDOUBLE, 20.0, 0.0, 100.0, false );
	glow = addParameter( "glow", tr("Glow:"), Parameter::PDOUBLE, 1.0, 0.0, 10.0, true );
	highlight = addParameter( "highlight", tr("Highlight:"), Parameter::PDOUBLE, 0.2, 0.0, 1.0, false );

	GLMask::setParameters();
}



QString GLGlow::getDescriptor( double pts, Frame *src, Profile *p  )
{
	return QString("%1 %2").arg( getIdentifier() ).arg( GLMask::getMaskDescriptor(pts, src, p) );
}



bool GLGlow::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Effect *e = el[0];
	bool ok = e->set_float( "radius", getParamValue( radius, pts ).toFloat() )
		&& e->set_float( "blurred_mix_amount", getParamValue( glow, pts ).toFloat() )
		&& e->set_float( "highlight_cutoff", getParamValue( highlight, pts ).toFloat() );
	ok |= GLMask::processMask(pts, src, p);

	return ok;
}



QList<Effect*> GLGlow::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new PseudoEffect(this, new GlowEffect) );
	return list;
}
