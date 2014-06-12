#include <movit/unsharp_mask_effect.h>
#include "vfx/glsharpen.h"



GLSharpen::GLSharpen( QString id, QString name ) : GLFilter( id, name )
{
	amount = 0.3;
	addParameter( tr("Amount:"), PFLOAT, 0.0, 2.0, true, &amount );
}



GLSharpen::~GLSharpen()
{
}



bool GLSharpen::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return e->set_float( "amount", amount );
}



Effect* GLSharpen::getMovitEffect()
{
	return new UnsharpMaskEffect();
}
