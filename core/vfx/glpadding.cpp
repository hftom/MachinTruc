#include <movit/padding_effect.h>
#include "vfx/glpadding.h"



GLPadding::GLPadding( QString id, QString name ) : GLFilter( id, name )
{
}



GLPadding::~GLPadding()
{
}



bool GLPadding::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	float left = 0;//( p->getVideoWidth() - src->glWidth ) / 2.0;
	float top = 0;//( p->getVideoHeight() - src->glHeight ) / 2.0;
	return e->set_int( "width", p->getVideoWidth() )
		&& e->set_int( "height", p->getVideoHeight() )
		&& e->set_float( "top", top )
		&& e->set_float( "left", left );
}



Effect* GLPadding::getMovitEffect()
{
	return new PaddingEffect();
}
