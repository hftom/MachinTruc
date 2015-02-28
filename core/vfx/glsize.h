#ifndef GLSIZE_H
#define GLSIZE_H

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "glsoftborder.h"


static const char *MyRotateEffect_shader=
"uniform vec2 PREFIX(factor);\n"
"uniform vec2 PREFIX(cosSin);\n"
"uniform vec2 PREFIX(offset);\n"
"uniform vec2 PREFIX(scale);\n"
"uniform vec2 PREFIX(centerOffset);\n"
"\n"
"vec4 FUNCNAME( vec2 tc ) {\n"
"	tc -= PREFIX(offset);\n"
"	tc *= PREFIX(scale);\n"
"	tc -= PREFIX(centerOffset);\n"
"	tc *= PREFIX(factor);\n"
"	tc *= mat2( PREFIX(cosSin).x, PREFIX(cosSin).y, -PREFIX(cosSin).y, PREFIX(cosSin).x );\n"
"	tc /= PREFIX(factor);\n"
"	tc += PREFIX(centerOffset);\n"
"	return INPUT( tc );\n"
"}\n";



class MyRotateEffect : public Effect {
public:
	MyRotateEffect() : owidth(1), oheight(1), iwidth(1), iheight(1), top(0), left(0), centerOffsetX(0), centerOffsetY(0), angle(0.0), SAR(1.0) {
		register_float("angle", &angle);
		register_float("SAR", &SAR);
		register_int("width", &owidth);
		register_int("height", &oheight);
		register_float("top", &top);
		register_float("left", &left);
		register_float("centerOffsetX", &centerOffsetX);
		register_float("centerOffsetY", &centerOffsetY);
	}
	
	virtual std::string effect_type_id() const { return "MyRotateEffect"; }
	std::string output_fragment_shader() { return MyRotateEffect_shader; }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual bool changes_output_size() const { return true; }
	
	virtual void get_output_size(unsigned *width, unsigned *height, unsigned *virtual_width, unsigned *virtual_height) const {
		*virtual_width = *width = owidth;
		*virtual_height = *height = oheight;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		//Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		Q_UNUSED( sampler_num );
		float offset[2] = { left / owidth, ( oheight - iheight - top ) / oheight };
		set_uniform_vec2( glsl_program_num, prefix, "offset", offset );

		float scale[2] = { float(owidth) / iwidth, float(oheight) / iheight };
		set_uniform_vec2( glsl_program_num, prefix, "scale", scale );
		
		float centerOffset[2] = { 0.5f + (centerOffsetX / iwidth), 0.5f + (centerOffsetY / iheight) };
		set_uniform_vec2( glsl_program_num, prefix, "centerOffset", centerOffset );
	
		// we have to take both texture_size_ratio and sample_aspect_ratio into account.
		float factor[2] = { SAR * iwidth / iheight, 1.0f };
		set_uniform_vec2( glsl_program_num, prefix, "factor", factor );
		
		float cosSin[2] = { (float)cos( angle ),  (float)sin( angle ) };
		set_uniform_vec2( glsl_program_num, prefix, "cosSin", cosSin );
	}

private:
	int owidth, oheight;
	float iwidth, iheight;
	float top, left;
	float centerOffsetX, centerOffsetY;
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
	double centerOffsetX, centerOffsetY;
	
	Parameter *xOffsetPercent, *yOffsetPercent;
	double left, top;
};

#endif //GLSIZE_H
