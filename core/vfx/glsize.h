#ifndef GLSIZE_H
#define GLSIZE_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "glsoftborder.h"


static const char *MyRotateEffect_shader=
"uniform vec2 PREFIX(factor);\n"
"uniform vec2 PREFIX(cosSin);\n"
"\n"
"vec4 FUNCNAME( vec2 tc ) {\n"
"	tc -= vec2( 0.5 );\n"
"	tc *= PREFIX(factor);\n"
"	tc *= mat2( PREFIX(cosSin).x, PREFIX(cosSin).y, -PREFIX(cosSin).y, PREFIX(cosSin).x );\n"
"	tc /= PREFIX(factor);\n"
"	tc += vec2( 0.5 );\n"
"	return INPUT( tc );\n"
"}\n";



class MyRotateEffect : public Effect {
public:
	MyRotateEffect() : iwidth(1), iheight(1), angle(0.0), SAR(1.0) {
		register_float("angle", &angle);
		register_float("SAR", &SAR);
	}
	
	virtual std::string effect_type_id() const { return "MyRotateEffect"; }
	std::string output_fragment_shader() { return MyRotateEffect_shader; }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		//Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		Q_UNUSED( sampler_num );		
		// we have to take both texture_size_ratio and sample_aspect_ratio into account.
		float factor[2] = { SAR * iwidth / iheight, 1.0f };
		set_uniform_vec2( glsl_program_num, prefix, "factor", factor );
		
		float cosSin[2] = { (float)cos( angle ),  (float)sin( angle ) };
		set_uniform_vec2( glsl_program_num, prefix, "cosSin", cosSin );
	}

private:
	float iwidth, iheight;
	float angle;
	float SAR;
};



class GLSize : public GLFilter
{
public:
	GLSize( QString id, QString name );
	~GLSize();
 
	QString getDescriptor( Frame *src, Profile *p );
	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();

		
private:
	void findPoints( double &x1, double &x2, double first, double last );
	
	Parameter *sizePercent;
	bool resizeActive;
	int resizeOutputWidth, resizeOutputHeight;
	double resizeZoomX, resizeZoomY;
	double resizeLeft, resizeTop;
	double resizeZoomCenterX, resizeZoomCenterY;
	
	Parameter *rotateAngle, *softBorder;
	bool rotateActive;
	double rotateLeft, rotateTop;
	double rotateSize;
	
	Parameter *xOffsetPercent, *yOffsetPercent;
	double left, top;
};

#endif //GLSIZE_H
