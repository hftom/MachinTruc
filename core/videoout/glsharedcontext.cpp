#include "glsharedcontext.h"



GLSharedContext::GLSharedContext( QOpenGLContext *shared ) : QOpenGLContext()
{
	setShareContext( shared );
	create();
	surface.setFormat( format() );
	surface.create();
}



bool GLSharedContext::makeCurrent()
{
	return QOpenGLContext::makeCurrent( &surface );
}
