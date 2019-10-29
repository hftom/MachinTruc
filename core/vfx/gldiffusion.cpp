#include <movit/diffusion_effect.h>
#include "vfx/gldiffusion.h"



GLDiffusion::GLDiffusion( QString id, QString name ) : GLFilter( id, name )
{
	mixAmount = addParameter( "mixAmount", tr("Amount:"), Parameter::PDOUBLE, 0.7, 0.0, 1.0, true );
	blurRadius = addParameter( "blurRadius", tr("Blur:"), Parameter::PDOUBLE, 1.4, 0.0, 1.5, false );
}



GLDiffusion::~GLDiffusion()
{
}



bool GLDiffusion::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return el.at(0)->set_float( "blurred_mix_amount", getParamValue( mixAmount, pts ).toFloat() )
		&& el.at(0)->set_float( "radius", src->glWidth * getParamValue( blurRadius, pts ).toDouble() / 100.0 );
}



QList<Effect*> GLDiffusion::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new DiffusionEffect() );
	return list;
}
