#include <movit/vignette_effect.h>
#include "vfx/glvignette.h"



GLVignette::GLVignette( QString id, QString name ) : GLFilter( id, name )
{
    center[0] = 0.5;
	center[1] = 0.5;
	radius = 0.5;
	inner_radius = 0.3;
}



GLVignette::~GLVignette()
{
}



bool GLVignette::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p )
	return e->set_vec2( "center", center )
		&& e->set_float( "radius", radius )
		&& e->set_float( "inner_radius", inner_radius );
}



Effect* GLVignette::getMovitEffect()
{
	return new VignetteEffect();
}
