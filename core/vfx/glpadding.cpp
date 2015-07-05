#include <movit/padding_effect.h>
#include "vfx/glpadding.h"



GLPadding::GLPadding( QString id, QString name ) : GLFilter( id, name ),
	left( 0 ),
	top( 0 )
{
}



GLPadding::~GLPadding()
{
}



void GLPadding::preProcess( Frame *src, Profile *p )
{
	left = (p->getVideoWidth() - src->glWidth) / 2.0;
	top = (p->getVideoHeight() - src->glHeight) / 2.0;
	src->glWidth = p->getVideoWidth();
	src->glHeight = p->getVideoHeight();
}



QString GLPadding::getDescriptor( double pts, Frame *src, Profile *p )
{
	Q_UNUSED( pts );
	preProcess( src, p );
	return getIdentifier();
}




bool GLPadding::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( pts );
	preProcess( src, p );	
	Effect* e = el[0];
	return e->set_int( "width", src->glWidth )
		&& e->set_int( "height", src->glHeight )
		&& e->set_float( "top", top )
		&& e->set_float( "left", left );
}



QList<Effect*> GLPadding::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new PaddingEffect() );
	return list;
}
