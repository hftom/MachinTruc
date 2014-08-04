#include <movit/lift_gamma_gain_effect.h>
#include <math.h>
#include "vfx/glliftgammagain.h"



GLLiftGammaGain::GLLiftGammaGain( QString id, QString name ) : GLFilter( id, name )
{
	liftR = addParameter( tr("Lift red:"), Parameter::PDOUBLE, 0.0, 0.0, 1.0, true );
	liftG = addParameter( tr("Lift green:"), Parameter::PDOUBLE, 0.0, 0.0, 1.0, true );
	liftB = addParameter( tr("Lift blue:"), Parameter::PDOUBLE, 0.0, 0.0, 1.0, true );
	gammaR = addParameter( tr("Gamma red:"), Parameter::PDOUBLE, 1.0, 0.0, 5.0, true );
	gammaG = addParameter( tr("Gamma green:"), Parameter::PDOUBLE, 1.0, 0.0, 5.0, true );
	gammaB = addParameter( tr("Gamma blue:"), Parameter::PDOUBLE, 1.0, 0.0, 5.0, true );
	gainR = addParameter( tr("Gain red:"), Parameter::PDOUBLE, 1.0, 0.0, 5.0, true );
	gainG = addParameter( tr("Gain green:"), Parameter::PDOUBLE, 1.0, 0.0, 5.0, true );
	gainB = addParameter( tr("Gain blue:"), Parameter::PDOUBLE, 1.0, 0.0, 5.0, true );
}



GLLiftGammaGain::~GLLiftGammaGain()
{
}



bool GLLiftGammaGain::process( const QList<Effect*> &el, Frame *src, Profile *p )
{
	Q_UNUSED( p );
	double pts = src->pts();
	float lift[3] = { (float)getParamValue( liftR, pts ), (float)getParamValue( liftG, pts ), (float)getParamValue( liftB, pts ) };
	float gamma[3] = { (float)getParamValue( gammaR, pts ), (float)getParamValue( gammaG, pts ), (float)getParamValue( gammaB, pts ) };
	float gain[3] = { (float)getParamValue( gainR, pts ), (float)getParamValue( gainG, pts ), (float)getParamValue( gainB, pts ) };
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
