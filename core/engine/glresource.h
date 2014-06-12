// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#ifndef GLRESOURCE_H
#define GLRESOURCE_H

#define GL_GLEXT_PROTOTYPES

#include <movit/effect_chain.h>
//#include <QGLShaderProgram>

class Frame;


class GLOBJECT
{
public:
    GLOBJECT( int w, int h );
    int width();
    int height();
    bool isFree();
    void setFree( bool b );
    bool isValid();

protected:
    int pwidth;
    int pheight;
    bool used;
    bool valid;
};



class FBO : public GLOBJECT
{
public:
    FBO( int w, int h );
    ~FBO();
    GLuint fbo();

private:
    GLuint fb;
};



class RBO : public GLOBJECT
{
public:
    RBO( int w, int h );
    ~RBO();
    GLuint rbo();

private:
    GLuint rb;
};



class TEXTURE : public GLOBJECT
{
public:
    TEXTURE( int w, int h, GLint iformat );
    ~TEXTURE();
    GLuint texture();
    GLint internalFormat();

private:
    GLuint tex;
    GLint pinternalFormat;
};



class FENCE
{
public:
	FENCE() {
		sync = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );
		used = true;
	}
	~FENCE() {
		glDeleteSync( sync );
	}
	GLsync fence() { return sync; }
	void setFree() { used = false; }
	bool isFree() { return !used; }

private:
	GLsync sync;
	bool used;
};



class GLResource
{
public:
    GLResource();
    ~GLResource();

    FBO* getFBO( int width, int height );
    void releaseFBO( FBO* f );
    RBO* getRBO( int width, int height );
    void releaseRBO( RBO* r );
    TEXTURE* getTexture( int width, int height, GLint iformat );
    void releaseTexture( TEXTURE *t );
    GLuint getPBO( int size );
	FENCE* getFence();

    void setOrthoView( int width, int height );
    void drawQuad( float x1, float y1, float x2, float y2 );

    bool black( Frame *dst );

private:
    QList<FBO*> fboList;
    QList<RBO*> rboList;
    QList<TEXTURE*> textureList;
	QList<FENCE*> fences;

    GLuint videoPBO;
    int videoPBOSize;
};

#endif //GLRESOURCE_H
