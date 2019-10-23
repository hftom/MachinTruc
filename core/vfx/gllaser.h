#ifndef GLLASER_H
#define GLLASER_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



static const char *MyLaserEffect_shader=
"float PREFIX(makePoint)( float x, float y, float fx, float fy, float sx, float sy ) {\n"
"   float t = PREFIX(time) / 10.0;\n"
"   float xx = 1.0 * ( x + sin( t * fx ) * cos( t * sx ) / 0.2 );\n"
"   float yy = 1.0 * ( y + cos( t * fy ) * sin( t * sy ) / 0.3);\n"
"   return 0.4 / sqrt( abs( xx * yy ) );\n"
"}\n"
"\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"   vec2 p = vec2(tc.x * PREFIX(iwidth), tc.y * PREFIX(iheight)) / PREFIX(iwidth) * 2.0 - vec2( 1.0, PREFIX(iheight) / PREFIX(iwidth) );\n"
"   p = p * 3.0;\n"
"   float x = p.x;\n"
"   float y = p.y;\n"
"\n"
"   float a = PREFIX(makePoint)( x, y, 3.3, 2.9, 1.3, 0.3 );\n"
"   a += PREFIX(makePoint)( x, y, 1.9, 2.0, 0.4, 0.4 );\n"
"   a += PREFIX(makePoint)( x, y, 0.2, 0.7, 0.4, 0.5 );\n"
"\n"
"   float b = PREFIX(makePoint)( x, y, 1.2, 1.9, 0.3, 0.3 );\n"
"   b += PREFIX(makePoint)( x, y, 0.7, 2.7, 0.4, 4.0 );\n"
"   b += PREFIX(makePoint)( x, y, 1.4, 0.6, 0.4, 0.5 );\n"
"   b += PREFIX(makePoint)( x, y, 2.6, 0.4, 0.6, 0.3 );\n"
"   b += PREFIX(makePoint)( x, y, 0.1, 1.4, 0.5, 0.4 );\n"
"   b += PREFIX(makePoint)( x, y, 0.7, 1.7, 0.4, 0.4 );\n"
"   b += PREFIX(makePoint)( x, y, 0.8, 0.5, 0.4, 0.5 );\n"
"   b += PREFIX(makePoint)( x, y, 1.4, 0.9, 0.6, 0.3 );\n"
"   b += PREFIX(makePoint)( x, y, 0.7, 1.3, 0.5, 0.4 );\n"
"\n"
"   float c = PREFIX(makePoint)( x, y, 3.7, 0.3, 0.3, 0.3 );\n"
"   c += PREFIX(makePoint)( x, y, 1.9, 1.3, 0.4, 0.4 );\n"
"   c += PREFIX(makePoint)( x, y, 0.8, 0.9, 0.4, 0.5 );\n"
"   c += PREFIX(makePoint)( x, y, 1.2, 1.7, 0.6, 0.3 );\n"
"   c += PREFIX(makePoint)( x, y, 0.3, 0.6, 0.5, 0.4 );\n"
"   c += PREFIX(makePoint)( x, y, 0.3, 0.3, 0.4, 0.4 );\n"
"   c += PREFIX(makePoint)( x, y, 1.4, 0.8, 0.4, 0.5 );\n"
"   c += PREFIX(makePoint)( x, y, 0.2, 0.6, 0.6, 0.3 );\n"
"   c += PREFIX(makePoint)( x, y, 1.3, 0.5, 0.5, 0.4 );\n"
"\n"
"   vec3 d = vec3( b * c, a * c, a * b ) / 500.0;\n"
"   return clamp(vec4( d, 1.0 ), vec4(0.0), vec4(1.0));\n"
"}\n";



class MyLaserEffect : public Effect {
public:
	MyLaserEffect() : time(0), iterations(2), iwidth(1), iheight(1) {
		register_float("time", &time);
		register_float("iterations", &iterations);
		register_float("iwidth", &iwidth);
		register_float("iheight", &iheight);
	}
	std::string effect_type_id() const { return "MyLaserEffect"; }
	std::string output_fragment_shader() { return MyLaserEffect_shader; }
	
private:
	float time, iterations, iwidth, iheight;
};



class GLLaser : public GLFilter
{
public:
	GLLaser( QString id, QString name );

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *iterations;
};

#endif // GLLASER_H
