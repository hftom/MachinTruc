#include <movit/lift_gamma_gain_effect.h>
#include <math.h>
#include "vfx/glliftgammagain.h"



GLLiftGammaGain::GLLiftGammaGain( QString id, QString name ) : GLFilter( id, name )
{
	liftR = addParameter( "liftR", tr("Lift red:"), Parameter::PDOUBLE, 0.0, 0.0, 1.0, true );
	liftG = addParameter( "liftG", tr("Lift green:"), Parameter::PDOUBLE, 0.0, 0.0, 1.0, true );
	liftB = addParameter( "liftB", tr("Lift blue:"), Parameter::PDOUBLE, 0.0, 0.0, 1.0, true );
	gammaR = addParameter( "gammaR", tr("Gamma red:"), Parameter::PDOUBLE, 1.0, 0.0, 5.0, true );
	gammaG = addParameter( "gammaG", tr("Gamma green:"), Parameter::PDOUBLE, 1.0, 0.0, 5.0, true );
	gammaB = addParameter( "gammaB", tr("Gamma blue:"), Parameter::PDOUBLE, 1.0, 0.0, 5.0, true );
	gainR = addParameter( "gainR", tr("Gain red:"), Parameter::PDOUBLE, 1.0, 0.0, 5.0, true );
	gainG = addParameter( "gainG", tr("Gain green:"), Parameter::PDOUBLE, 1.0, 0.0, 5.0, true );
	gainB = addParameter( "gainB", tr("Gain blue:"), Parameter::PDOUBLE, 1.0, 0.0, 5.0, true );
}



GLLiftGammaGain::~GLLiftGammaGain()
{
}



bool GLLiftGammaGain::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	double pts = src->pts();
	float lift[3] = { getParamValue( liftR, pts ).toFloat(), getParamValue( liftG, pts ).toFloat(), getParamValue( liftB, pts ).toFloat() };
	float gamma[3] = { getParamValue( gammaR, pts ).toFloat(), getParamValue( gammaG, pts ).toFloat(), getParamValue( gammaB, pts ).toFloat() };
	float gain[3] = { getParamValue( gainR, pts ).toFloat(), getParamValue( gainG, pts ).toFloat(), getParamValue( gainB, pts ).toFloat() };
	Effect *e = el[0];
	return e->set_vec3( "lift", lift )
		&& e->set_vec3( "gamma", gamma )
		&& e->set_vec3( "gain", gain );
}



QList<Effect*> GLLiftGammaGain::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new LiftGammaGainEffect() );
	return list;
}
