#include <movit/diffusion_effect.h>
#include "vfx/gldiffusion.h"



GLDiffusion::GLDiffusion( QString id, QString name ) : GLFilter( id, name )
{
	mixAmount = 0.3;
	blurRadius = 3.0;
	addParameter( tr("Amount:"), PFLOAT, 0.0, 1.0, true, &mixAmount );
	addParameter( tr("Radius:"), PFLOAT, 0.0, 10.0, true, &blurRadius );
}



GLDiffusion::~GLDiffusion()
{
}



bool GLDiffusion::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return e->set_float( "blurred_mix_amount", mixAmount )
		&& e->set_float( "radius", blurRadius );
}



Effect* GLDiffusion::getMovitEffect()
{
	return new DiffusionEffect();
}
