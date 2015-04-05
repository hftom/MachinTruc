#ifndef GLTEXT_H
#define GLTEXT_H

#include <QImage>

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "glfilter.h"

static const char *MyTextEffect_shader=
"uniform sampler2D PREFIX(string_tex);\n"
"uniform vec2 PREFIX(offset);\n"
"uniform vec2 PREFIX(scale);\n"
"vec4 FUNCNAME( vec2 tc ) {\n"
"	vec4 background = INPUT( tc );\n"
"	tc -= PREFIX(offset);\n"
"	tc *= PREFIX(scale);\n"
"	vec4 text = tex2D( PREFIX(string_tex), vec2( tc.x, 1.0 - tc.y ) );\n"
"	return text + (1.0 - text.a) * background;\n"
"}\n";



class MyTextEffect : public Effect {
public:
	MyTextEffect() : iwidth(1), iheight(1), 
		currentImage( NULL ), reload( 1 )
	{
		register_int( "reload", &reload );
		
		glGenTextures( 1, &texnum );
	}
	
	~MyTextEffect() {
		glDeleteTextures( 1, &texnum );
	}
	
	virtual std::string effect_type_id() const { return "MyTextEffect"; }
	std::string output_fragment_shader() { return MyTextEffect_shader; }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Effect::set_gl_state( glsl_program_num, prefix, sampler_num );
		
		float w;
		float h;
		if ( currentImage ) {
			w = currentImage->width();
			h = currentImage->height();
		}
		else
			w = h = 1;
		
		float left = (iwidth - w) / 2.0f;
		float top = (iheight - h) / 2.0f;
		float offset[2] = { left / iwidth, ( iheight - h - top ) / iheight };
		set_uniform_vec2( glsl_program_num, prefix, "offset", offset );

		float scale[2] = { iwidth / w, iheight / h };
		set_uniform_vec2( glsl_program_num, prefix, "scale", scale );
		
		if ( currentImage && reload ) {
			glActiveTexture( GL_TEXTURE0 + *sampler_num );
			glBindTexture( GL_TEXTURE_2D, texnum );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, currentImage->constBits() );
			reload = 0;
		}
		
		glActiveTexture( GL_TEXTURE0 + *sampler_num );
		glBindTexture( GL_TEXTURE_2D, texnum );
		set_uniform_int( glsl_program_num, prefix, "string_tex", *sampler_num );
		++*sampler_num;
	}
	
	void setImage( QImage *img ) {
		if ( img != currentImage )
			reload = 1;
		currentImage = img;
	}

private:
	float iwidth, iheight;
	
	GLuint texnum;
	QImage *currentImage;
	int reload;
};



class GLText : public GLFilter
{
public:
	GLText( QString id, QString name );
	
	bool process( const QList<Effect*> &el, Frame *src, Profile *p );

	QList<Effect*> getMovitEffects();
	
private:
	Parameter *editor;
	QImage image;
	QString currentText;
};

#endif // GLTEXT_H
