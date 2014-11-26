// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#ifndef METRONOM_H
#define METRONOM_H

#include "audioout/ao_sdl.h"
#include "engine/frame.h"

#include <QGLWidget>
#include <QThread>
#include <QMutex>




class Metronom : public QThread
{
	Q_OBJECT
public:
	Metronom();
	~Metronom();
	void setRenderMode( bool b );
	void play( bool b, bool backward = false );
	void changeSpeed( int s );
	Frame* getLastFrame();
	void flush();

	static void readData( Frame **data, double time, void *userdata );

	void setSharedContext( QGLWidget *shared );

	MQueue<Frame*> videoFrames;
	MQueue<Frame*> encodeVideoFrames;
	MQueue<Frame*> freeVideoFrames;
	MQueue<Frame*> audioFrames;
	MQueue<Frame*> freeAudioFrames;

public slots:
	void setLastFrame( Frame *f );

private:
	void run();
	void runRender();
	void runShow();

	int speed;
	bool playBackward;
	bool running;
	double sclock, videoLate;
	QMutex clockMutex;
	AudioOutSDL ao;

	QGLWidget *fencesContext;

	bool renderMode;
	Frame *lastFrame;
	QMutex lastFrameMutex;

signals:
	void newFrame( Frame* );
	void currentFramePts( double );
	void discardFrame( int );

	void osdMessage( const QString &text, int duration );
};

#endif // METRONOM_H
