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



bool GLPadding::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	float left = xoffsetpercent * p->getVideoWidth() / 100.0;
	float top = yoffsetpercent * p->getVideoHeight() / 100.0;
	src->glWidth = p->getVideoWidth();
	src->glHeight = p->getVideoHeight();
	return el.at(0)->set_int( "width", p->getVideoWidth() )
		&& el.at(0)->set_int( "height", p->getVideoHeight() )
		&& el.at(0)->set_float( "top", top )
		&& el.at(0)->set_float( "left", left );
}



QList<Effect*> GLPadding::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new PaddingEffect() );
	return list;
}
