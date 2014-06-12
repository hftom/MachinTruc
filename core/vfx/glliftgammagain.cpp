#include <movit/lift_gamma_gain_effect.h>
#include <math.h>
#include "vfx/glliftgammagain.h"



GLLiftGammaGain::GLLiftGammaGain( QString id, QString name ) : GLFilter( id, name )
{
	lift[0] = lift[1] = lift[2] = 0.0;
	gamma[0] = gamma[1] = gamma[2] = 1.0;
	gain[0] = gain[1] = gain[2] = 1.0;
	addParameter( tr("Lift red:"), PFLOAT, 0.0, 1.0, true, &lift[0] );
	addParameter( tr("Lift green:"), PFLOAT, 0.0, 1.0, true, &lift[1] );
	addParameter( tr("Lift blue:"), PFLOAT, 0.0, 1.0, true, &lift[2] );
	addParameter( tr("Gamma red:"), PFLOAT, 0.0, 5.0, true, &gamma[0] );
	addParameter( tr("Gamma green:"), PFLOAT, 0.0, 5.0, true, &gamma[1] );
	addParameter( tr("Gamma blue:"), PFLOAT, 0.0, 5.0, true, &gamma[2] );
	addParameter( tr("Gain red:"), PFLOAT, 0.0, 5.0, true, &gain[0] );
	addParameter( tr("Gain green:"), PFLOAT, 0.0, 5.0, true, &gain[1] );
	addParameter( tr("Gain blue:"), PFLOAT, 0.0, 5.0, true, &gain[2] );
}



GLLiftGammaGain::~GLLiftGammaGain()
{
}



bool GLLiftGammaGain::process( Effect *e, Frame *src, Profile *p )
{
	Q_UNUSED( src );
	Q_UNUSED( p );
	return e->set_vec3( "lift", lift )
		&& e->set_vec3( "gamma", gamma )
		&& e->set_vec3( "gain", gain );
}



Effect* GLLiftGammaGain::getMovitEffect()
{
	return new LiftGammaGainEffect();
}
