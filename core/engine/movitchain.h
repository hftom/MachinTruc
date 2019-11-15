#ifndef MOVITCHAIN_H
#define MOVITCHAIN_H

#define GL_GLEXT_PROTOTYPES

#include <movit/effect_chain.h>
#include <movit/resource_pool.h>
#include <movit/effect.h>
#include <movit/input.h>

#include "engine/frame.h"
#include "vfx/glfilter.h"



using namespace movit;



static const char *Blank_input=
"vec4 FUNCNAME(vec2 tc) {\n"
"	return vec4( 0.0 );\n"
"}\n";


static const char *OpticalFiber_input=
"vec4 FUNCNAME( vec2 tc ) {\n"
"	tc -= 0.5;\n"
"	float vertColor = 0.0;\n"
"	float t = PREFIX(time) * 0.5;\n"
"	for( float i = 0.0; i < 4.0; i++ ) {\n"
"		tc.y += sin( t + tc.x * 6.0 ) * 0.45;\n"
"		tc.x += sin( -t + tc.y * 3.0 ) * 0.25;\n"
"		float value = atan( tc.y * 2.5 ) + sin( tc.x * 10.0 );\n"
"		float stripColor = 1.0 / sqrt( abs( value ) );\n"
"		vertColor += stripColor / 14.0 / 4.0;\n"
"	}\n"
"	return clamp( vec4( vec3( vertColor * 0.7, vertColor / 15.0, vertColor / 20.0 ), 1.0 ), vec4(0.0), vec4(1.0) );\n"
"}\n";


static const char *LaserGrid_input=
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


static const char *Warp_input=
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


static const char *Warp2_input=
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


static const char *Clouds_input=
"float PREFIX(hash)(float n) { return fract(pow(sin(n), 1.0) * 1e6); }\n"
"\n"
"float PREFIX(snoise)(vec2 p) {\n"
"	p *= 2.5;\n"
"	const vec2 d = vec2(1.0 ,30.0);\n"
"	vec2 f = fract(p), p0 = floor(p) * d, p1 = p0 + d;\n"
"	f.xy = f.xy * f.xy * (3.0 - 2.0 * f.xy);\n"
"	float t = mix(mix(PREFIX(hash)(p0.x + p0.y), PREFIX(hash)(p1.x + p0.y), f.x),\n"
"		       mix(PREFIX(hash)(p0.x + p1.y), PREFIX(hash)(p1.x + p1.y), f.x), f.y);\n"
"	return t * 2.0 - 1.0;\n"
"}\n"
"\n"
"float PREFIX(fbm)( vec2 p) {\n"
"	float f = 0.0;\n"
"	f += 0.5000 * PREFIX(snoise)(p); p *= 2.22;\n"
"	f += 0.2500 * PREFIX(snoise)(p); p *= 2.03;\n"
"	f += 0.1250 * PREFIX(snoise)(p); p *= 2.01;\n"
"	f += 0.0625 * PREFIX(snoise)(p); p *= 2.04;\n"
"	return (f / 0.9375);\n"
"}\n"
"\n"
"vec3 PREFIX(sun)( vec2 pos ) {\n"
"	vec2 p = vec2(0.75, 0.9);\n"
"	return vec3(2.5, 0.5, 0.0) / (distance( p, pos ) * 25.0);\n"
"}\n"
"\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	vec2 p = tc * 2.0 - 1.0;\n"
"	p.x /= PREFIX(iheight) / PREFIX(iwidth);\n"
"	vec3 c1 = mix(vec3(-0.5), vec3(0.0, 0.2, 1.0), tc.y);\n"
"	float c2 = PREFIX(fbm)(p - PREFIX(time) / 10.0) + PREFIX(fbm)(p - PREFIX(time) / 20.0) + PREFIX(fbm)(p - PREFIX(time) / 60.0) + 11.0;\n"
"	float v = c2 * 0.06; //(((c2 * 0.1) - 0.35) * 0.45) + 0.3;\n"
"	vec4 col = vec4( mix( c1, vec3(v), v ), 1.0 );\n"
"	return vec4( col.rgb + PREFIX(sun)( tc ), 1.0);\n"
"}\n";



class GLSLInput : public Input
{
public:
	GLSLInput( int w, int h, QString shader ) : iwidth(1), iheight(1), width( w ), height( h ), shaderName(shader), time(0), output_linear_gamma(true), needs_mipmaps(false) {
		register_float("iwidth", &iwidth);
		register_float("iheight", &iheight);
		register_float("time", &time);
		register_int("output_linear_gamma", &output_linear_gamma);
		register_int("needs_mipmaps", &needs_mipmaps);
		if (shaderName.isEmpty()) {
			shaderName = "Blank";
		}
	}
	std::string effect_type_id() const { return shaderName.toStdString(); }
	std::string output_fragment_shader() {
		if (shaderName == "OpticalFiber") return OpticalFiber_input;
		if (shaderName == "LaserGrid") return LaserGrid_input;
		if (shaderName == "Warp") return Warp_input;
		if (shaderName == "Warp2") return Warp2_input;
		if (shaderName == "Clouds") return Clouds_input;
		return Blank_input;
	}
	AlphaHandling alpha_handling() const { return INPUT_AND_OUTPUT_PREMULTIPLIED_ALPHA; }
	bool can_output_linear_gamma() const { return true; }
	unsigned get_width() const { return width; }
	unsigned get_height() const { return height; }
	Colorspace get_color_space() const { return COLORSPACE_sRGB; }
	GammaCurve get_gamma_curve() const { return GAMMA_LINEAR; }
	
private:
	float iwidth, iheight;
	int width, height;
	QString shaderName;
	float time;
	int output_linear_gamma, needs_mipmaps;
};



class MovitInput
{
public:
	MovitInput();
	~MovitInput();

	bool process( Frame *src, double pts, GLResource *gl = NULL );
	Input* getMovitInput( Frame *src );

	static QString getDescriptor( Frame *src );

private:
	bool setBuffer( PBO *p, Frame *src, int size );
	void setPixelData8(Frame *src, int size, int stride[], GLResource *gl );
	void setPixelData16(Frame *src, int size,int stride[],  GLResource *gl );
	Input *input;
	qint64 mmi;
	QString mmiProvider;
};



class MovitFilter
{
public:
	MovitFilter( const QList<Effect*> &el, GLFilter *f = NULL );
	
	QList<Effect*> effects;
	QSharedPointer<GLFilter> filter;
};



class MovitBranch
{
public:
	MovitBranch( MovitInput *in );
	~MovitBranch();
	
	MovitInput *input;
	QList<MovitFilter*> filters;
	MovitFilter *overlay;
};



class MovitChain
{
public:
	MovitChain();	
	~MovitChain();
	void reset();
	
	EffectChain *chain;
	QList<MovitBranch*> branches;
	
	QStringList descriptor;
};

#endif //MOVITCHAIN_H
