#include "vfx/glwater.h"



GLWater::GLWater( QString id, QString name ) : GLFilter( id, name )
{
}



GLWater::~GLWater()
{
}



bool GLWater::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p )
	return e->set_float( "time", src->pts() / MICROSECOND )
		&& e->set_float( "inwidth", src->glWidth )
		&& e->set_float( "inheight", src->glHeight );
}



Effect* GLWater::getMovitEffect()
{
	return new WaterEffect();
}