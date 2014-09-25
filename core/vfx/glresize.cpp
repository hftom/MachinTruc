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
	if ( src->glWidth < 1 )
		src->glWidth = 1;
	src->glSAR = p->getVideoSAR();
}



QString GLResize::getDescriptor( Frame *src, Profile *p )
{
	preProcess( src, p );
	return getIdentifier();
}



bool GLResize::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	preProcess( src, p );
	
	return el[0]->set_int( "width", src->glWidth )
		&& el[0]->set_int( "height", src->glHeight );
}



QList<Effect*> GLResize::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new ResampleEffect() );
	return list;
}
