#include <math.h>

#include <QtGlobal>

#include "util.h"



double nearestPTS( double pts, double frameDuration )
{
	qint64 len = ( pts + frameDuration / 2.0 ) / frameDuration;
	return (double)len * frameDuration;
}



double linearInterpolate( double y1, double y2, double x )
{
	return (y1 * (1.0 - x)) + (y2 * x);
}



double cosineInterpolate( double y1, double y2, double x )
{
	double x2 = (1.0 - cos(x * M_PI)) / 2.0;
	return (y1 * (1.0 - x2)) + (y2 * x2);
}



double sRgbToLinear( double colorComponent )
{
	if ( colorComponent < 0.04045 )
		return colorComponent / 12.92;
	else
		return pow( (colorComponent + 0.055) / 1.055, 2.4 );
}



void sRgbColorToLinear( QColor &color )
{
	color.setRedF( sRgbToLinear( color.redF() ) );
	color.setGreenF( sRgbToLinear( color.greenF() ) );
	color.setBlueF( sRgbToLinear( color.blueF() ) );
}



void sRgbColorToPremultipliedLinear( QColor &color )
{
	double a = color.alphaF();
	color.setRedF( sRgbToLinear( color.redF() ) * a );
	color.setGreenF( sRgbToLinear( color.greenF() ) * a );
	color.setBlueF( sRgbToLinear( color.blueF() ) * a );
}
