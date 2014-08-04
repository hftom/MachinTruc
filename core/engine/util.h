#ifndef UTIL_H
#define UTIL_H



double nearestPTS( double pts, double frameDuration );
double linearInterpolate( double y1, double y2, double x );
double cosineInterpolate( double y1, double y2, double x );

#endif // UTIL_H