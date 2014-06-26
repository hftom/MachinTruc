#ifndef GLWATER_H
#define GLWATER_H

#include <movit/effect.h>

#include "vfx/glfilter.h"


static const char *water_frag=
"const float PI = 3.1415926535897932;\n"
"\n"
"//speed\n"
"const float speed = 0.2;\n"
"const float speed_x = 0.3;\n"
"const float speed_y = 0.3;\n"
"\n"
"// geometry\n"
"const float intensity = 3.0;\n"
"const int steps = 8;\n"
"const float frequency = 4.0;\n"
"const int angle = 7; // better when a prime\n"
"\n"
"// reflection and emboss\n"
"const float delta = 220.0;\n"
"const float intence = 700.0;\n"
"const float emboss = 0.4;\n"
"\n"
"// crystals effect\n"
"float col( vec2 coord ) {\n"
"	float delta_theta = 2.0 * PI / float( angle );\n"
"	float col = 0.0;\n"
"	float theta = 0.0;\n"
"	for ( int i = 0; i < steps; i++ ) {\n"
"		vec2 adjc = coord;\n"
"		theta = delta_theta * float( i );\n"
"		adjc.x += cos( theta ) * PREFIX(time) * speed + PREFIX(time) * speed_x;\n"
"		adjc.y -= sin( theta ) * PREFIX(time) * speed - PREFIX(time) * speed_y;\n"
"		col = col + cos( ( adjc.x * cos( theta ) - adjc.y * sin( theta ) ) * frequency ) * intensity;\n"
"	}\n"
"	return cos( col );\n"
"}\n"
"\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	vec2 p = tc, c1 = p, c2 = p;\n"
"	float cc1 = col( c1 );\n"
"	c2.x += PREFIX(inwidth) / delta;\n"
"	float dx = emboss * ( cc1 - col( c2 ) ) / delta;\n"
"	c2.x = p.x;\n"
"	c2.y -= PREFIX(inheight) / delta;\n"
"	float dy = emboss * ( cc1 - col( c2 ) ) / delta;\n"
"	c1.x += dx;\n"
"	c1.y += dy;\n"
"	float alpha = 1.0 + dot( dx, dy ) * intence;\n"
"	vec4 org;\n"
"	//if ( c1.x < 0.0 || c1.x > 1.0 || c1.y < 0.0 || c1.y > 1.0 ) org = vec4(0);\n"
"	/*else*/ { org = INPUT( c1 ); org.rgb *= alpha; }\n"
"	return org;\n"
"}\n";



class WaterEffect : public Effect {
public:
	WaterEffect() {
		time = 0.0;
		inwidth = 1280;
		inheight = 720;
		register_float( "time", &time );
		register_float( "inwidth", &inwidth );
		register_float( "inheight", &inheight );
	}
	virtual std::string effect_type_id() const { return "WaterEffect"; }
	std::string output_fragment_shader() { return water_frag; }

private:
	float time, inwidth, inheight;
};


class GLWater : public GLFilter
{
public:
    GLWater( QString id, QString name );
    ~GLWater();

    bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
};

#endif //GLWATER_H