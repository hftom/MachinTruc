#ifndef UTIL_H
#define UTIL_H

#include <QColor>



double nearestPTS( double pts, double frameDuration );
double linearInterpolate( double y1, double y2, double x );
double cosineInterpolate( double y1, double y2, double x );
double sRgbToLinear( double colorComponent );
void sRgbColorToLinear( QColor &color );
void sRgbColorToPremultipliedLinear( QColor &color );

#endif // UTIL_H