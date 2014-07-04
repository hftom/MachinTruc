#include <movit/padding_effect.h>
#include "vfx/glpadding.h"



GLPadding::GLPadding( QString id, QString name ) : GLFilter( id, name )
{
	xoffsetpercent = 0.0;
	yoffsetpercent = 0.0;
	addParameter( tr("X:"), PFLOAT, -100.0, 100.0, true, &xoffsetpercent );
	addParameter( tr("Y:"), PFLOAT, -100.0, 100.0, true, &yoffsetpercent );
}



GLPadding::~GLPadding()
{
}



bool GLPadding::preProcess( Frame *src, Profile *p )
{
	left = (p->getVideoWidth() - src->glWidth) / 2.0;
	top = (p->getVideoHeight() - src->glHeight) / 2.0;
	src->glWidth = p->getVideoWidth();
	src->glHeight = p->getVideoHeight();
	src->paddingAuto = false;
	left += xoffsetpercent * src->glWidth / 100.0;
	top += yoffsetpercent * src->glHeight / 100.0;
	
	return true;
}




bool GLPadding::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	preProcess( src, p );
	
	return el.at(0)->set_int( "width", src->glWidth )
		&& el.at(0)->set_int( "height", src->glHeight )
		&& el.at(0)->set_float( "top", top )
		&& el.at(0)->set_float( "left", left );
}



QList<Effect*> GLPadding::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new PaddingEffect() );
	return list;
}
