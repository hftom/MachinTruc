// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include <math.h>

#include "engine/frame.h"
#include "engine/glresource.h"



bool GLResource::black( Frame *dst )
{
	int w = dst->profile.getVideoWidth();
	int h = dst->profile.getVideoHeight();

	FBO *fbo = getFBO( w, h, GL_RGBA );
	if ( !fbo )
		return false;

	glBindFramebuffer( GL_FRAMEBUFFER, fbo->fbo() );
	glClear( GL_COLOR_BUFFER_BIT );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	dst->setFBO( fbo );
	return true;
}



PBO* GLResource::getPBO( int size )
{
	int i;
	int supindex = -1, infindex = -1;
	int supdelta = 0, infdelta = 0;

	for ( i = 0; i < pboList.count(); ++i ) {
		PBO *p = pboList.at(i);
		if ( p->isFree() ) {
			if ( p->size() == size ) {
				p->setFree( false );
				return p;
			}
			if ( p->size() > size ) {
				int d = p->size() - size;
				if ( supindex == -1 ) {
					supdelta = d;
					supindex = i;
				}
				else if ( d < supdelta ) {
					supdelta = d;
					supindex = i;
				}
			}
			else {
				int d = size - p->size();
				if ( infindex == -1 ) {
					infdelta = d;
					infindex = i;
				}
				else if ( d < infdelta ) {
					infdelta = d;
					infindex = i;
				}
			}
		}
	}

	if ( supindex != -1 ) {
		PBO *p = pboList.at( supindex );
		p->setFree( false );
		return p;
	}

	if ( infindex != -1 ) {
		PBO *p = pboList.at( infindex );
		if ( !p->resize( size ) ) {
			qDebug() << "pbo resize error";
			return NULL;
		}
		p->setFree( false );
		return p;
	}

	PBO *p = new PBO( size );
	if ( !p->isValid() ) {
		delete p;
		qDebug() << "new pbo error";
		return NULL;
	}

	pboList.append( p );
	p->setFree( false );
	return p;
}



FBO* GLResource::getFBO( int width, int height, GLint iformat )
{
	int i;

	for ( i = 0; i < fboList.count(); ++i ) {
		FBO *f = fboList.at(i);
		if ( f->isFree() && ( f->width() != width || f->height() != height || f->format() != iformat ) ) {
			delete fboList.takeAt( i-- );
		}
	}

	for ( i = 0; i < fboList.count(); ++i ) {
		FBO *f = fboList.at(i);
		if ( f->isFree() && f->width() == width && f->height() == height && f->format() == iformat ) {
			f->setFree( false );
			return f;
		}
	}

	FBO *f = new FBO( width, height, iformat );
	if ( !f->isValid() ) {
		delete f;
		return NULL;
	}

	fboList.append( f );
	f->setFree( false );
	return f;
}



FENCE* GLResource::getFence()
{
	int i;
	for ( i = 0; i < fences.count(); ++i ) {
		FENCE *f = fences.at( i );
		if ( f->isFree() )
			delete fences.takeAt( i-- );
	}
	FENCE *f = new FENCE();
	fences.append( f );
	return f;
}



void GLResource::setOrthoView( int width, int height )
{
	glViewport( 0, 0, width, height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0.0, width, 0.0, height, -1.0, 1.0 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}



void GLResource::drawQuad( float x1, float y1, float x2, float y2 )
{
	glBegin( GL_QUADS );
		glTexCoord2f( x1, y1 );		glVertex3f( x1, y1, 0.);
		glTexCoord2f( x1, y2 );		glVertex3f( x1, y2, 0.);
		glTexCoord2f( x2, y2 );		glVertex3f( x2, y2, 0.);
		glTexCoord2f( x2, y1 );		glVertex3f( x2, y1, 0.);
	glEnd();
}



// FBO
FBO::FBO( int w, int h, GLint iformat )
{
	tex = 0;
	fb = 0;

	valid = init( w, h, iformat );
}



bool FBO::init( int w, int h, GLint iformat )
{
	iw = w;
	ih = h;
	internalFormat = iformat;

	glGenFramebuffers( 1, &fb );
	if ( !fb )
		return false;

	glGenTextures( 1, &tex );
	if ( !tex )
		return false;

	glBindTexture( GL_TEXTURE_2D, tex );
	glTexImage2D( GL_TEXTURE_2D, 0, iformat, iw, ih, 0, iformat, GL_UNSIGNED_BYTE, NULL );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glBindTexture( GL_TEXTURE_2D, 0 );

	glBindFramebuffer( GL_FRAMEBUFFER, fb );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0 );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	return true;
}



FBO::~FBO()
{
	if ( tex )
		glDeleteTextures( 1, &tex );
	if ( fb )
		glDeleteFramebuffers( 1, &fb );
}



// PBO
PBO::PBO( int sz )
{
	pb = 0;
	isize = 0;
	used = false;
	valid = init( sz );
}



bool PBO::init( int sz )
{
	glGenBuffers( 1, &pb );
	if ( !pb )
		return false;

	return resize( sz );
}



bool PBO::resize( int sz )
{
	glGetError();
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, pb );
	glBufferData( GL_PIXEL_UNPACK_BUFFER_ARB, sz, NULL, GL_STREAM_DRAW );
	glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0 );

	if ( GL_NO_ERROR != glGetError() )
		return false;

	isize = sz;
	return true;
}



PBO::~PBO()
{
	if ( pb )
		glDeleteBuffers( 1, &pb );
}
