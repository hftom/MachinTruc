#ifndef GLSHADOW_H
#define GLSHADOW_H

#include <movit/effect_util.h>

#include "vfx/glfilter.h"



static const char *shadowshader=
"vec4 FUNCNAME(vec2 tc) {\n"
"	float x = 1.0 / PREFIX(width) * PREFIX(xoffset);\n"
"	float y = 1.0 / PREFIX(height) * PREFIX(yoffset);\n"
"	vec4 col = INPUT( tc );\n"
"	float a = INPUT( tc - vec2(x, -y) ).a;\n"
"	return col + (1.0 - col.a) * (vec4(0.0, 0.0, 0.0, PREFIX(opacity)) * a);\n"
"}\n";



class MyShadowEffect : public Effect {
public:
	MyShadowEffect() : opacity( 0.6 ), xoffset( 10.0 ), yoffset( 10.0 ), width( 1280.0 ), height( 720.0 ) {
		register_float( "opacity", &opacity );
		register_float( "xoffset", &xoffset );
		register_float( "yoffset", &yoffset );
		register_float( "width", &width );
		register_float( "height", &height );
	}
	std::string effect_type_id() const { return "MyShadowEffect"; }
	std::string output_fragment_shader() { return shadowshader; }

private:
	float opacity;
	float xoffset, yoffset;
	float width, height;
};



class GLShadow : public GLFilter
{
public:
    GLShadow( QString id, QString name );
    ~GLShadow();
	
	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	float opacity;
	float xoffset, yoffset;
};

#endif // GLSHADOW_H
