#ifndef GLKALEIDOSCOPE_H
#define GLKALEIDOSCOPE_H

#include <movit/effect.h>
#include <movit/effect_util.h>

#include "vfx/glfilter.h"

// Simple Kaleidoscope. (c) Inigo Quilez 2009.
static const char *KaleidoscopeEffect_shader=
"vec4 FUNCNAME(vec2 tc)\n"
"{\n"
"    vec2 p = -1.0 + 2.0 * tc;\n"
"    float a = atan(p.y, p.x);\n"
"    float r = sqrt(dot(p, p));\n"
"    vec2 uv;\n"
"    uv.x = PREFIX(size) * a / 3.1416;\n"
"    uv.y = -PREFIX(time) + sin(PREFIX(size) * r + PREFIX(time)) + 0.7 * cos(PREFIX(time) + PREFIX(size) * a);\n"
"    float w = 0.5 + 0.5 * (sin(PREFIX(time) + PREFIX(size) * r) + 0.7 * cos(PREFIX(time) + PREFIX(size) * a));\n"
"    vec4 col = INPUT(mod(uv, 1.0));\n"
"\n"
"    return vec4(col.rgb * w, col.a);\n"
"}\n";



class KaleidoscopeEffect : public Effect {
public:
	KaleidoscopeEffect() : time(0.0), size(7.0)
	{
		register_float( "time", &time );
		register_float( "size", &size );
	}

	virtual std::string effect_type_id() const { return "KaleidoscopeEffect"; }
	std::string output_fragment_shader() { return KaleidoscopeEffect_shader; }

private:
	float time, size;
};



class GLKaleidoscope : public GLFilter
{
	Q_OBJECT
public:
	GLKaleidoscope( QString id, QString name );
	~GLKaleidoscope();

	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *size;
};

#endif //GLKALEIDOSCOPE_H
