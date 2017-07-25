#ifndef GLSHAREDCOONTEXT_H
#define GLSHAREDCOONTEXT_H
#include <QOpenGLContext>
#include <QOffscreenSurface>



class GLSharedContext : public QOpenGLContext
{
public:
	GLSharedContext( QOpenGLContext *shared );
	bool makeCurrent();

private:
	QOffscreenSurface surface;
};

#endif // GLSHAREDCOONTEXT_H
