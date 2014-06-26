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



bool GLVignette::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p )
	return el.at(0)->set_vec2( "center", center )
		&& el.at(0)->set_float( "radius", radius )
		&& el.at(0)->set_float( "inner_radius", inner_radius );
}



QList<Effect*> GLVignette::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new VignetteEffect() );
	return list;
}
