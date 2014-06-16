#include <movit/resample_effect.h>
#include <movit/resize_effect.h>
#include "vfx/glresize.h"



GLResize::GLResize( QString id, QString name ) : GLFilter( id, name )
{
	width = height = 16;
	percent = 100.0;
	addParameter( tr("Size:"), PFLOAT, 0.01, 100.0, true, &percent );
}



GLResize::~GLResize()
{
}



bool GLResize::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	//src->glWidth = width;
	//src->glHeight = height;
	width = (float)src->profile.getVideoWidth() * percent / 100.0;
	if ( width < 1 ) width = 1;
	height = (float)src->profile.getVideoHeight() * width / (float)src->profile.getVideoWidth();
	if ( height < 1 ) height = 1;
	return e->set_int( "width", width )
		&& e->set_int( "height", height );
}



Effect* GLResize::getMovitEffect()
{
	return new ResampleEffect();
}
