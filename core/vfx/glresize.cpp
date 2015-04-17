#include <movit/resample_effect.h>
#include <movit/resize_effect.h>
#include "vfx/glresize.h"



GLResize::GLResize( QString id, QString name ) : GLFilter( id, name )
{
}



GLResize::~GLResize()
{
}



void GLResize::preProcess( Frame *src, Profile *p )
{
	src->glWidth = (double)src->glWidth * src->glSAR  / p->getVideoSAR();
	src->glSAR = p->getVideoSAR();
	double r = (double)src->glWidth / (double)src->glHeight;
	if ( r >= (double)p->getVideoWidth() / (double)p->getVideoHeight() ) {
		src->glWidth = p->getVideoWidth();
		src->glHeight = (double)src->glWidth / r;
	}
	else {
		src->glHeight = p->getVideoHeight();
		src->glWidth = r * (double)src->glHeight;
	}
	if ( src->glWidth < 1 )
		src->glWidth = 1;
	if ( src->glHeight < 1 )
		src->glHeight = 1;
}



QString GLResize::getDescriptor( Frame *src, Profile *p )
{
	preProcess( src, p );
	return getIdentifier();
}



bool GLResize::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	int glw = src->glWidth;
	int glh = src->glHeight;
	
	preProcess( src, p );

	if ( src->glOVD )
		src->glOVDTransformList.append( FilterTransform( FilterTransform::SCALE, (double)src->glWidth / glw, (double)src->glHeight / glh ) );
	
	return el[0]->set_int( "width", src->glWidth )
		&& el[0]->set_int( "height", src->glHeight );
}



QList<Effect*> GLResize::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new ResampleEffect() );
	return list;
}
