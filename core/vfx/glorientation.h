#ifndef GLORIENTATION_H
#define GLORIENTATION_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "glsoftborder.h"



static const char *MyOrientationEffect_shader=
"vec4 FUNCNAME( vec2 tc ) {\n"
"#ifdef ANGLE_90\n"
"	return INPUT( vec2(1.0 - tc.y, tc.x) );\n"
"#endif\n"
"#ifdef ANGLE_270\n"
"	return INPUT( vec2(tc.y, 1.0 - tc.x) );\n"
"#endif\n"
"	return INPUT( vec2(1.0 - tc.x, 1.0 - tc.y) );\n"
"#undef ANGLE_90\n"
"#undef ANGLE_270\n"
"}\n";



class MyOrientationEffect : public Effect {
public:
	MyOrientationEffect() : iwidth(1), iheight(1), angle(0) {
		register_int( "angle", &angle );
	}
	
	std::string effect_type_id() const { return "MyOrientationEffect"; }
	
	std::string output_fragment_shader() { 
		QString s = MyOrientationEffect_shader;
		if ( angle == 90 )
			return s.prepend( "#define ANGLE_90 1\n" ).toLatin1().data();
		if ( angle == 270 )
			return s.prepend( "#define ANGLE_270 1\n" ).toLatin1().data();
		return s.toLatin1().data();
	}
	
	bool needs_linear_light() const { return false; }
	bool needs_srgb_primaries() const { return false; }
	AlphaHandling alpha_handling() const { return DONT_CARE_ALPHA_TYPE; }
	bool changes_output_size() const { return angle != 180; }
	
	void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	void get_output_size(unsigned *width, unsigned *height,
						unsigned *virtual_width, unsigned *virtual_height) const {
		if ( angle == 180 ) {
			*width = *virtual_width = iwidth;
			*height = *virtual_height = iheight;
		}
		else {
			*width = *virtual_width = iheight;
			*height = *virtual_height = iwidth;
		}
	}

private:
	float iwidth, iheight;
	int angle;
};



class GLOrientation : public GLFilter
{
public:
	GLOrientation( QString id = "AutoRotate", QString name = "AutoRotate" );
	~GLOrientation();
 
	QString getDescriptor( double pts, Frame *src, Profile *p );
	bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );

	void setOrientation( int a );
	QList<Effect*> getMovitEffects();
	
private:
	int angle;
};

#endif //GLORIENTATION_H
