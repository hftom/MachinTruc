#include <movit/resample_effect.h>
#include <movit/resize_effect.h>
#include "vfx/glresize.h"



GLResize::GLResize( QString id, QString name ) : GLFilter( id, name )
{
	width = height = 16;
}



GLResize::~GLResize()
{
}



bool GLResize::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	src->glWidth = width;
	src->glHeight = height;
	return e->set_int( "width", width )
		&& e->set_int( "height", height );
}



Effect* GLResize::getMovitEffect()
{
	return new ResampleEffect();
}
