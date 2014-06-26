#include <movit/resample_effect.h>
#include <movit/resize_effect.h>
#include "vfx/glresize.h"



GLResize::GLResize( QString id, QString name ) : GLFilter( id, name )
{
	width = height = 16;
	percent = 100.0;
	addParameter( tr("Size:"), PFLOAT, 0.01, 500.0, true, &percent );
}



GLResize::~GLResize()
{
}



bool GLResize::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	width = (float)src->profile.getVideoWidth() * percent / 100.0;
	if ( width < 1 ) width = 1;
	height = (float)src->profile.getVideoHeight() * width / (float)src->profile.getVideoWidth();
	if ( height < 1 ) height = 1;
	src->glWidth = width;
	src->glHeight = height;
	return el.at(0)->set_int( "width", width )
		&& el.at(0)->set_int( "height", height );
}



QList<Effect*> GLResize::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new ResampleEffect() );
	return list;
}
