// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#ifndef GLRESOURCE_H
#define GLRESOURCE_H

#define GL_GLEXT_PROTOTYPES

#include <movit/effect_chain.h>



class Frame;



class FBO
{
public:
	FBO( int w, int h, GLint iformat );
	~FBO();
	int width() { return iw; }
	int height() { return ih; }
	GLint format() { return internalFormat; }
	GLuint fbo() { return fb; }
	GLuint texture() { return tex; }
	void setFree( bool b ) { used = !b; }
	bool isFree() { return !used; }
	bool isValid() { return valid; }

private:
	bool init( int w, int h, GLint iformat );

	int iw, ih;
	GLuint fb;
	GLuint tex;
	GLint internalFormat;
	bool used, valid;
};



class PBO
{
public:
	PBO( int sz );
	~PBO();
	bool resize( int sz );
	int size() { return isize; }
	GLuint pbo() { return pb; }
	void setFree( bool b ) { used = !b; }
	bool isFree() { return !used; }
	bool isValid() { return valid; }

private:
	bool init( int sz );

	int isize;
	GLuint pb;
	bool used, valid;
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
	FBO* getFBO( int width, int height, GLint iformat );
	PBO* getPBO( int size );
	FENCE* getFence();

	bool black( Frame *dst );

private:
	QList<FBO*> fboList;
	QList<PBO*> pboList;
	QList<FENCE*> fences;
};

#endif //GLRESOURCE_H
