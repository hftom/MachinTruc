#include <QtGlobal>

#include "util.h"



double nearestPTS( double pts, double frameDuration )
{
	qint64 len = ( pts + frameDuration / 2.0 ) / frameDuration;
	return (double)len * frameDuration;
}
