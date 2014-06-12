// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include <math.h>

#include "engine/frame.h"
#include "engine/glresource.h"



GLResource::GLResource()
{
    videoPBO = 0;
    videoPBOSize = 0;
}



GLResource::~GLResource()
{
}



bool GLResource::black( Frame *dst )
{
    int w = dst->profile.getVideoWidth();
    int h = dst->profile.getVideoHeight();

    FBO *fbo = getFBO( w, h );
    if ( !fbo )
        return false;

    TEXTURE *dest = getTexture( w, h, GL_RGBA );
    if ( !dest ) {
        releaseFBO( fbo );
        return false;
    }

    glBindFramebuffer( GL_FRAMEBUFFER, fbo->fbo() );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dest->texture(), 0 );
    glClear( GL_COLOR_BUFFER_BIT );

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    releaseFBO( fbo );

    dst->setTexture( dest );
    return true;
}



GLuint GLResource::getPBO( int size )
{
    if ( !videoPBO ) {
        glGenBuffers( 1, &videoPBO );
        if ( !videoPBO )
            return 0;
    }

    if ( size > videoPBOSize ) {
        glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, videoPBO );
        glBufferData( GL_PIXEL_UNPACK_BUFFER_ARB, size, NULL, GL_STREAM_DRAW );
        glBindBuffer( GL_PIXEL_UNPACK_BUFFER_ARB, 0 );
        videoPBOSize = size;
    }

    return videoPBO;
}



FBO* GLResource::getFBO( int width, int height )
{
    int i;

    //printf("FBO: %d, TEXTURE: %d, SHADER: %d\n", fboList.count(), textureList.count(), shaderList.count() );

    for ( i = 0; i < fboList.count(); ++i ) {
        FBO *f = fboList.at(i);
        if ( f->isFree() && (f->width() == width) && (f->height() == height) ) {
            f->setFree( false );
            return f;
        }
    }

    FBO *f = new FBO( width, height );
    if ( !f->isValid() ) {
        delete f;
        return NULL;
    }

    fboList.append( f );
    return f;
}



void GLResource::releaseFBO( FBO* f )
{
    f->setFree( true );
}



RBO* GLResource::getRBO( int width, int height )
{
    int i;

    for ( i = 0; i < rboList.count(); ++i ) {
        RBO *r = rboList.at(i);
        if ( r->isFree() && (r->width() == width) && (r->height() == height) ) {
            r->setFree( false );
            return r;
        }
    }

    RBO *r = new RBO( width, height );
    if ( !r->isValid() ) {
        delete r;
        return NULL;
    }

    rboList.append( r );
    return r;
}



void GLResource::releaseRBO( RBO* r )
{
    r->setFree( true );
}



TEXTURE* GLResource::getTexture( int width, int height, GLint iformat )
{
    int i;

    for ( i = 0; i < textureList.count(); ++i ) {
        TEXTURE *t = textureList.at(i);
        if ( t->isFree() && (t->width() == width) && (t->height() == height) && (t->internalFormat() == iformat) ) {
            glBindTexture( GL_TEXTURE_2D, t->texture() );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glBindTexture( GL_TEXTURE_2D, 0 );
            t->setFree( false );
            return t;
        }
    }

    TEXTURE *t = new TEXTURE( width, height, iformat );
    if ( !t->isValid() ) {
        delete t;
        return NULL;
    }

    textureList.append( t );
    return t;
}



void GLResource::releaseTexture( TEXTURE *t )
{
    t->setFree( true );
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



///////////// GLOBJECT ///////////////
GLOBJECT::GLOBJECT( int w, int h )
{
    pwidth = w;
    pheight = h;
    used = true;
}

int GLOBJECT::width()
{
    return pwidth;
}

int GLOBJECT::height()
{
    return pheight;
}

bool GLOBJECT::isFree()
{
    return !used;
}

void GLOBJECT::setFree( bool b )
{
    used = !b;
}

bool GLOBJECT::isValid()
{
    return valid;
}



///////////// FBO ///////////////
FBO::FBO( int w, int h ) : GLOBJECT( w, h )
{
    glGenFramebuffers( 1, &fb );
    if ( !fb )
        valid = false;
    else
        valid = true;
}

FBO::~FBO()
{
    glDeleteFramebuffers( 1, &fb );
}

GLuint FBO::fbo()
{
    return fb;
}



///////////// RBO ///////////////
RBO::RBO( int w, int h ) : GLOBJECT( w, h )
{
    glGenRenderbuffers( 1, &rb );
    if ( !rb )
        valid = false;
    else {
        valid = true;
        glBindRenderbuffer( GL_RENDERBUFFER, rb );
        glRenderbufferStorageMultisample( GL_RENDERBUFFER, 0, GL_RGBA, w, h );
    }
}

RBO::~RBO()
{
    glDeleteRenderbuffers( 1, &rb );
}

GLuint RBO::rbo()
{
    return rb;
}



///////////// TEXTURE ///////////////
TEXTURE::TEXTURE( int w, int h, GLint iformat ) : GLOBJECT( w, h )
{
    pinternalFormat = iformat;
    glGenTextures( 1, &tex );
    if ( !tex )
        valid = false;
    else {
        valid = true;
        glBindTexture( GL_TEXTURE_2D, tex );
        glTexImage2D( GL_TEXTURE_2D, 0, iformat, w, h, 0, iformat, GL_UNSIGNED_BYTE, NULL );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glBindTexture( GL_TEXTURE_2D, 0 );
    }
}

TEXTURE::~TEXTURE()
{
    glDeleteTextures( 1, &tex );
}

GLuint TEXTURE::texture()
{
    return tex;
}

GLint TEXTURE::internalFormat()
{
    return pinternalFormat;
}
