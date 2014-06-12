#include <movit/blur_effect.h>
#include "vfx/glblur.h"



GLBlur::GLBlur( QString id, QString name ) : GLFilter( id, name )
{
	radius = 4.0;
	addParameter( tr("Radius:"), PFLOAT, 0.0, 100.0, true, &radius );
}



GLBlur::~GLBlur()
{
}



bool GLBlur::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return e->set_float( "radius", radius );
}



Effect* GLBlur::getMovitEffect()
{
	return new BlurEffect();
}
