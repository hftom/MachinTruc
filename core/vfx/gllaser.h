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


static const char *warp_frag=
"float PREFIX(hash)( float n ) {\n"
"    return fract(sin(n) * 9.0);\n"
"}\n"
"\n"
"float PREFIX(noise)( in vec2 x ) {\n"
"    vec2 p = floor(x);\n"
"    vec2 f = fract(x);\n"
"    f = f * f * (3.0 - 2.0 * f);\n"
"    float n = p.x + p.y * 7919.0;\n"
"    return mix(mix( PREFIX(hash)(n + 0.0), PREFIX(hash)(n + 1.0), f.x), mix( PREFIX(hash)(n + 7919.0), PREFIX(hash)(n + 7920.0),f.x),f.y);\n"
"}\n"
"\n"
"float PREFIX(fbm)( vec2 p ) {\n"
"    return (0.25 * PREFIX(noise)( p * 3.0 )) / 0.38375;\n"
"}\n"
"\n"
"vec4 FUNCNAME( vec2 tc ) {\n"
"	vec2 p = -1.0 + 2.0 * tc;\n"
"	p.x *= PREFIX(iwidth) / PREFIX(iheight);\n"
"\n"
"	vec2 dx1 = vec2(1.0, 1.0);\n"
"	vec2 dy1 = vec2(1.2, 1.3);\n"
"\n"
"	vec2 dx2 = vec2(1.6, 1.3);\n"
"	vec2 dy2 = vec2(1.2, 1.6);\n"
"\n"
"	dx1 = dx1 * mat2(cos(PREFIX(time) / 5.33), -sin(PREFIX(time) / 2.66), -sin(PREFIX(time) / 2.33), cos(PREFIX(time) / 3.33));\n"
"\n"
"	vec2 q = vec2( PREFIX(fbm)( p + dx1 ), PREFIX(fbm)( p + dy1 ) );\n"
"	vec2 r = vec2( PREFIX(fbm)( p + 1.5 * q + dx2 ), PREFIX(fbm)( p + 1.5 * q + dy2 ) );\n"
"	vec2 s = vec2( PREFIX(fbm)( p + 1.5 * r + dx1 + dx2 ), PREFIX(fbm)( p + 1.5 * r + dy2 + dy2 ) );\n"
"\n"
"	float v = PREFIX(fbm)( p + 4. * s );\n"
"	vec3 col = v * vec3(q.x, r.x, s.x) + vec3(q.y, r.y, s.y);\n"
"	return vec4( col, 1. );\n"
"}\n";


static const char *warp2_frag=
"float PREFIX(hash)( float n ) {\n"
"    return fract(sin(n) * 9.0);\n"
"}\n"
"\n"
"float PREFIX(noise)( in vec2 x ) {\n"
"    vec2 p = floor(x);\n"
"    vec2 f = fract(x);\n"
"    f = f * f * (3.0 - 2.0 * f);\n"
"    float n = p.x + p.y * 7919.0;\n"
"    return mix(mix( PREFIX(hash)(n + 0.0), PREFIX(hash)(n + 1.0), f.x), mix( PREFIX(hash)(n + 7919.0), PREFIX(hash)(n + 7920.0),f.x),f.y);\n"
"}\n"
"\n"
"float PREFIX(fbm)( vec2 p ) {\n"
"    return (0.25 * PREFIX(noise)( p * 3.0 )) / 0.38375;\n"
"}\n"
"\n"
"vec4 FUNCNAME( vec2 tc ) {\n"
"	vec2 p = -1.0 + 2.0 * tc;\n"
"	p.x *= PREFIX(iwidth) / PREFIX(iheight);\n"
"\n"
"	vec2 dx1 = vec2(1.0, 1.0);\n"
"	vec2 dy1 = vec2(1.2, 1.3);\n"
"\n"
"	vec2 dx2 = vec2(1.6, 1.3);\n"
"	vec2 dy2 = vec2(1.2, 1.6);\n"
"\n"
"	dx1 = dx1 * mat2(cos(PREFIX(time) / 5.33), -sin(PREFIX(time) / 2.66), -sin(PREFIX(time) / 2.33), cos(PREFIX(time) / 3.33));\n"
"\n"
"	vec2 q = vec2( PREFIX(fbm)( p + dx1 ), PREFIX(fbm)( p + dy1 ) );\n"
"	vec2 s = vec2( PREFIX(fbm)( p + 3.5 * q + dx1 + dx2 ), PREFIX(fbm)( p + 3.5 * q + dy1 + dy2 ) );\n"
"\n"
"	float v = PREFIX(fbm)( p + 5.0 * s );\n"
"	vec3 col = 2.0 * v * vec3(q.x + s.y, q.y * s.x, s.x - s.y) + vec3(s.x + q.y, s.y * q.x, q.x - q.y);\n"
"	col = col * 0.4;\n"
"	return vec4( col, 1.0 );\n"
"}\n";



class MyLaserEffect : public Effect {
public:
	MyLaserEffect() : time(0), iwidth(1), iheight(1) {
		register_float("time", &time);
		register_float("iwidth", &iwidth);
		register_float("iheight", &iheight);
	}
	std::string effect_type_id() const { return "MyLaserEffect"; }
	std::string output_fragment_shader() { return MyLaserEffect_shader; }
	
private:
	float time, iwidth, iheight;
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
