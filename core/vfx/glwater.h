#ifndef GLWATER_H
#define GLWATER_H

#include <movit/effect.h>
#include <movit/effect_util.h>

#include "vfx/glfilter.h"

/* Simple Water shader. (c) Victor Korsun, bitekas@gmail.com; 2012.
   Attribution-ShareAlike CC License.
*/
static const char *water_frag=
"uniform vec2 PREFIX(size_div_delta);\n"
"\n"
"// crystals effect\n"
"const float PREFIX(delta_theta) = 2.0 * 3.1415926535897932 / 7.0;\n"
"float PREFIX(color)( vec2 coord ) {\n"
"	float col = 0.0;\n"
"	float theta = 0.0;\n"
"	for ( int i = 0; i < 8; i++ ) {\n"
"		vec2 adjc = coord;\n"
"		theta = PREFIX(delta_theta) * float( i );\n"
"		adjc.x += cos( theta ) * PREFIX(time) * PREFIX(speed);\n"
"		adjc.y -= sin( theta ) * PREFIX(time) * PREFIX(speed);\n"
"		col = col + cos( ( adjc.x * cos( theta ) - adjc.y * sin( theta ) ) * PREFIX(frequency) ) * PREFIX(intensity);\n"
"	}\n"
"	return cos( col );\n"
"}\n"
"\n"
"vec4 FUNCNAME(vec2 tc) {\n"
"	vec2 p = tc, c1 = p, c2 = p;\n"
"	float cc1 = PREFIX(color)( c1 );\n"
"	c2.x += PREFIX(size_div_delta).x;\n"
"	float dx = PREFIX(emboss) * ( cc1 - PREFIX(color)( c2 ) ) / PREFIX(delta);\n"
"	c2.x = p.x;\n"
"	c2.y -= PREFIX(size_div_delta).y;\n"
"	float dy = PREFIX(emboss) * ( cc1 - PREFIX(color)( c2 ) ) / PREFIX(delta);\n"
"	c1.x += dx;\n"
"	c1.y += dy;\n"
"	float alpha = 1.0 + dot( dx, dy ) * PREFIX(intence);\n"
"	vec4 result = INPUT( c1 );\n"
"	result.rgb *= alpha;\n"
"	return result;\n"
"}\n";



class WaterEffect : public Effect {
public:
	WaterEffect() {
		time = 0.0;
		inwidth = 1280;
		inheight = 720;
		speed = 0.1;
		emboss = 0.4;
		intensity = 3.0;
		frequency = 4.0;
		delta = 220.0;
		intence = 700.0;
		register_float( "time", &time );
		register_float( "speed", &speed );
		register_float( "emboss", &emboss );
		register_float( "intensity", &intensity );
		register_float( "frequency", &frequency );
		register_float( "delta", &delta );
		register_float( "intence", &intence );
	}

	virtual std::string effect_type_id() const { return "WaterEffect"; }
	std::string output_fragment_shader() { return water_frag; }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		inwidth = width;
		inheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		float size_div_delta[2] = { inwidth / delta, inheight / delta };
		set_uniform_vec2( glsl_program_num, prefix, "size_div_delta", size_div_delta );
	}

private:
	float time, inwidth, inheight;
	//speed
	float speed;
	// refraction
	float emboss, intensity, frequency;
	// reflection
	float delta, intence;
};



class GLWater : public GLFilter
{
public:
    GLWater( QString id, QString name );
    ~GLWater();

    bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	float speed;
	float emboss, intensity, frequency;
	float delta, intence;
};

#endif //GLWATER_H
