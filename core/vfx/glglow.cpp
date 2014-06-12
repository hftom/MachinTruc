#include <movit/glow_effect.h>
#include "vfx/glglow.h"



GLGlow::GLGlow( QString id, QString name ) : GLFilter( id, name )
{
	radius = 20.0;
	glow = 1.0;
	highlight = 0.2;	
	addParameter( tr("Radius:"), PFLOAT, 0.0, 100.0, true, &radius );
	addParameter( tr("Glow:"), PFLOAT, 0.0, 10.0, true, &glow );
	addParameter( tr("Highlight:"), PFLOAT, 0.0, 1.0, true, &highlight );
}



GLGlow::~GLGlow()
{
}



bool GLGlow::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return e->set_float( "radius", radius ) && e->set_float( "blurred_mix_amount", glow ) && e->set_float( "highlight_cutoff", highlight );
}



Effect* GLGlow::getMovitEffect()
{
	return new GlowEffect();
}
