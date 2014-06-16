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



bool GLPadding::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	float left = xoffsetpercent * p->getVideoWidth() / 100.0;// - src->glWidth ) / 2.0;
	float top = yoffsetpercent * p->getVideoHeight() / 100.0;//- src->glHeight ) / 2.0;
	return e->set_int( "width", p->getVideoWidth() )
		&& e->set_int( "height", p->getVideoHeight() )
		&& e->set_float( "top", top )
		&& e->set_float( "left", left );
}



Effect* GLPadding::getMovitEffect()
{
	return new PaddingEffect();
}
