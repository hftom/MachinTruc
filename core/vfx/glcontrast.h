#ifndef GLCONTRAST_H
#define GLCONTRAST_H

#include <movit/effect_util.h>

#include "vfx/glmask.h"



static const char *MContrastEffect_shader=
"vec4 FUNCNAME( vec2 tc ) {\n"
"	vec4 col = INPUT( tc );\n"
"	vec3 c = col.rgb / col.a;\n"
"	c = mix( c, vec3(0.5),  -PREFIX(contrast) / 20.0 );\n"
"	c += PREFIX(brightness) / 20.0;\n"
"	return vec4( clamp( c, 0.0, 1.0 ) * col.a, col.a );\n"
"}\n";



class MContrastEffect : public Effect {
public:
	MContrastEffect() : contrast(0.0), brightness(0.0) {
		register_float("contrast", &contrast);
		register_float("brightness", &brightness);
	}
	virtual std::string effect_type_id() const { return "MContrastEffect"; }
	std::string output_fragment_shader() { return MContrastEffect_shader; }

private:
	float contrast, brightness;
};



class GLContrast : public GLMask
{
public:
	GLContrast( QString id, QString name );
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	QString getDescriptor( double pts, Frame *src, Profile *p  );
	QList<Effect*> getMovitEffects();

private:
	Parameter *contrast, *brightness;
};

#endif //GLCONTRAST_H
