#include "vfx/gldeinterlace.h"



GLDeinterlace::GLDeinterlace( QString id, QString name ) : GLFilter( id, name )
{
}



GLDeinterlace::~GLDeinterlace()
{
}



bool GLDeinterlace::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	return e->set_float( "height", src->glHeight );
}



Effect* GLDeinterlace::getMovitEffect()
{
	return new MyDeinterlaceEffect();
}







