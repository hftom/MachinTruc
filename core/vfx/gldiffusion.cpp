#include <movit/diffusion_effect.h>
#include "vfx/gldiffusion.h"



GLDiffusion::GLDiffusion( QString id, QString name ) : GLFilter( id, name )
{
	mixAmount = addParameter( tr("Amount:"), Parameter::PDOUBLE, 0.3, 0.0, 1.0, true );
	blurRadius = addParameter( tr("Radius:"), Parameter::PDOUBLE, 3.0, 0.0, 10.0, true );
}



GLDiffusion::~GLDiffusion()
{
}



bool GLDiffusion::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	return el.at(0)->set_float( "blurred_mix_amount", getParamValue( mixAmount, src->pts() ) )
		&& el.at(0)->set_float( "radius", getParamValue( blurRadius, src->pts() ) );
}



QList<Effect*> GLDiffusion::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new DiffusionEffect() );
	return list;
}
