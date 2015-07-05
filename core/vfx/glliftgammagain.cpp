#include <movit/lift_gamma_gain_effect.h>
#include <movit/util.h>
#include <math.h>
#include "vfx/glliftgammagain.h"



GLLiftGammaGain::GLLiftGammaGain( QString id, QString name ) : GLFilter( id, name )
{
	lift = addParameter( "lift", tr("Shadows:"), Parameter::PCOLORWHEEL, QColor::fromRgbF( 0, 0, 0 ), QColor::fromRgbF( 0, 0, 0 ), QColor::fromRgbF( 1, 1, 1 ), false );
	lift->layout.setLayout( 0, 0 );
	gamma = addParameter( "gamma", tr("Midtones:"), Parameter::PCOLORWHEEL, QColor::fromRgbF( 0, 0, 0.5 ), QColor::fromRgbF( 0, 0, 0 ), QColor::fromRgbF( 1, 1, 1 ), false );
	gamma->layout.setLayout( 0, 1 );
	gain = addParameter( "gain", tr("Highlights:"), Parameter::PCOLORWHEEL, QColor::fromRgbF( 0, 0, 0.25 ), QColor::fromRgbF( 0, 0, 0 ), QColor::fromRgbF( 1, 1, 1 ), false );
	gain->layout.setLayout( 0, 2 );
}



GLLiftGammaGain::~GLLiftGammaGain()
{
}



bool GLLiftGammaGain::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( pts );
	Q_UNUSED( src );
	Q_UNUSED( p );
	QColor clift = getParamValue( lift ).value<QColor>();
	RGBTriplet liftRgb(0.0f, 0.0f, 0.0f);
	hsv2rgb_normalized( M_PI * 2 * clift.redF(), clift.greenF(), clift.blueF(), &liftRgb.r, &liftRgb.g, &liftRgb.b );
	
	QColor cgamma = getParamValue( gamma ).value<QColor>();
	RGBTriplet gammaRgb(0.0f, 0.0f, 0.0f);
	hsv2rgb_normalized( M_PI * 2 * cgamma.redF(), cgamma.greenF(), cgamma.blueF() * 2, &gammaRgb.r, &gammaRgb.g, &gammaRgb.b );
	
	QColor cgain = getParamValue( gain ).value<QColor>();
	RGBTriplet gainRgb(0.0f, 0.0f, 0.0f);
	hsv2rgb_normalized( M_PI * 2 * cgain.redF(), cgain.greenF(), cgain.blueF() * 4, &gainRgb.r, &gainRgb.g, &gainRgb.b );
	
	Effect *e = el[0];
	return e->set_vec3( "lift", (float*)&liftRgb )
		&& e->set_vec3( "gamma", (float*)&gammaRgb )
		&& e->set_vec3( "gain", (float*)&gainRgb );
}



QList<Effect*> GLLiftGammaGain::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new LiftGammaGainEffect() );
	return list;
}
