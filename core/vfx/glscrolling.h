#ifndef GLSCROLLING_H
#define GLSCROLLING_H

#include <QImage>

#include <movit/effect_util.h>
#include <movit/effect_chain.h>
#include <movit/util.h>

#include "glfilter.h"



class MyScrollingEffect : public Effect {
public:
	MyScrollingEffect() : iwidth(1), iheight(1), imgWidth(1), imgHeight(1), totalImageHeight(1),
		top(0), left(0), opacity(1), reload(true), currentImage( NULL )
	{
		register_float( "top", &top );
		register_float( "left", &left );
		register_float( "opacity", &opacity );
		glGenTextures( 1, &texnum );
		//setText( ".", 1, 1 );
	}
	
	~MyScrollingEffect() {
		glDeleteTextures( 1, &texnum );
		if (currentImage) {
			delete currentImage;
		}
	}
	
	virtual std::string effect_type_id() const { return "MyScrollingEffect"; }
	std::string output_fragment_shader() { return GLFilter::getShader("scrolling.frag"); }
	
	virtual void inform_input_size(unsigned, unsigned width, unsigned height) {
		iwidth = width;
		iheight = height;
	}
	
	virtual void set_gl_state( GLuint glsl_program_num, const std::string &prefix, unsigned *sampler_num ) {
		Effect::set_gl_state( glsl_program_num, prefix, sampler_num );

		if ( reload ) {
			glActiveTexture( GL_TEXTURE0 + *sampler_num );
			glBindTexture( GL_TEXTURE_2D, texnum );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, currentImage->constBits() );
			
			delete currentImage;
			currentImage = NULL;
			reload = false;
		}
		
		float oleft = ((iwidth - imgWidth) / 2.0f) + left;
		float otop = ((iheight - imgHeight) / 2.0f) + top;

		float offset[2] = { oleft / iwidth, ( iheight - imgHeight - otop ) / iheight };
		set_uniform_vec2( glsl_program_num, prefix, "offset", offset );

		float scale[2] = { iwidth / imgWidth, iheight / imgHeight };
		set_uniform_vec2( glsl_program_num, prefix, "scale", scale );
		
		glActiveTexture( GL_TEXTURE0 + *sampler_num );
		glBindTexture( GL_TEXTURE_2D, texnum );
		set_uniform_int( glsl_program_num, prefix, "string_tex", *sampler_num );
		++*sampler_num;
	}
	
	QImage* drawImage();

	void setText( const QString &text, int iw, int ih ) {
		if ( text != currentText ) {
			currentText = text;
			iwidth = iw;
			iheight = ih;
			if ( currentImage )
				delete currentImage;
			currentImage = drawImage();
			imgWidth = currentImage->width();
			imgHeight = currentImage->height();
			reload = true;
		}
	}
	
	float getImageWidth() { return imgWidth; }
	float getImageHeight() { return imgHeight; }

private:
	float iwidth, iheight;
	float imgWidth, imgHeight, totalImageHeight;
	float top, left;
	float opacity;
	
	GLuint texnum;
	QString currentText;
	bool reload;
	QImage *currentImage;
};



class GLScrolling : public GLFilter
{
	Q_OBJECT
public:
	GLScrolling( QString id, QString name );
	
	virtual bool process( const QList<Effect*> &el, double pts, Frame *src, Profile *p );
	
	QList<Effect*> getMovitEffects();
	
private:
	Parameter *editor;
	Parameter *opacity;
	Parameter *xOffset, *yOffset;
};

#endif // GLSCROLLING_H
