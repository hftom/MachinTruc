#ifndef GLDEINTERLACE_H
#define GLDEINTERLACE_H

#include "vfx/glfilter.h"



static const char *shader=
"vec4 FUNCNAME( vec2 tc ) {\n"
"	vec4 blend = 2.0 * INPUT( tc );\n"
"	blend += INPUT( vec2( tc.x, tc.y - (1.0 / PREFIX(height)) ) );\n"
"	blend += INPUT( vec2( tc.x, tc.y + (1.0 / PREFIX(height)) ) );\n"
"	return blend / 4.0;\n"
"}\n";



class MyDeinterlaceEffect : public Effect {
public:
	MyDeinterlaceEffect() : height(720.0) {
		register_float("height", &height);
	}
	virtual std::string effect_type_id() const { return "MyDeinterlaceEffect"; }
	std::string output_fragment_shader() { return shader; }
	virtual bool needs_texture_bounce() const { return true; }
	virtual AlphaHandling alpha_handling() const { return DONT_CARE_ALPHA_TYPE; }
	
private:
	float height;
};



class GLDeinterlace : public GLFilter
{
public:
    GLDeinterlace( QString id = "DeinterlaceAuto", QString name = "DeinterlaceAuto" );
    ~GLDeinterlace();

    bool process( Effect *e, Frame *src, Profile *p );

	Effect* getMovitEffect();
};

#endif // GLDEINTERLACE_H
