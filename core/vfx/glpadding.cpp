#include <movit/padding_effect.h>
#include "vfx/glpadding.h"



GLPadding::GLPadding( QString id, QString name ) : GLFilter( id, name )
{
	xoffsetpercent = addParameter( tr("X:"), Parameter::PDOUBLE, 0.0, -100.0, 100.0, true, "%" );
	yoffsetpercent = addParameter( tr("Y:"), Parameter::PDOUBLE, 0.0, -100.0, 100.0, true, "%" );
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
	double pts = src->pts();
	left += getParamValue( xoffsetpercent, pts ).toDouble() * src->glWidth / 100.0;
	top += getParamValue( yoffsetpercent, pts ).toDouble() * src->glHeight / 100.0;
}



QString GLPadding::getDescriptor( Frame *src, Profile *p )
{
	preProcess( src, p );
	return getIdentifier();
}




bool GLPadding::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
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
