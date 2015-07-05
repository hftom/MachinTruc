#include <movit/white_balance_effect.h>

#include "engine/util.h"
#include "vfx/glwhitebalance.h"



GLWhiteBalance::GLWhiteBalance( QString id, QString name ) : GLFilter( id, name )
{
	neutralColor = addParameter( "neutralColor", tr("Neutral color:"), Parameter::PRGBCOLOR, QColor::fromRgbF( 0.5, 0.5, 0.5 ), QColor::fromRgbF( 0, 0, 0 ), QColor::fromRgbF( 1, 1, 1 ), false );
	temperature = addParameter( "temperature", tr("Temperature:"), Parameter::PINT, 6500, 1000, 15000, false );
}



GLWhiteBalance::~GLWhiteBalance()
{
}



bool GLWhiteBalance::process( const QList<Effect*> &el, double pts, Frame *src, Profile *p )
{
	Q_UNUSED( pts );
	Q_UNUSED( p );
	Q_UNUSED( src );
	Effect *e = el[0];
	QColor c = getParamValue( neutralColor ).value<QColor>();
	// convert gamma and premultiply
	sRgbColorToLinear( c );
	RGBTriplet col = RGBTriplet( c.redF(), c.greenF(), c.blueF() );
	return e->set_vec3( "neutral_color", (float*)&col )
		&& e->set_float( "output_color_temperature", getParamValue( temperature ).toFloat() );
}



QList<Effect*> GLWhiteBalance::getMovitEffects()
{
	QList<Effect*> list;
	list.append( new WhiteBalanceEffect() );
	return list;
}
