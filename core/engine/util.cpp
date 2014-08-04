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
