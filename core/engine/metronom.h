// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#ifndef METRONOM_H
#define METRONOM_H

#include "audioout/ao_sdl.h"
#include "engine/playbackbuffer.h"

#include <QGLWidget>
#include <QThread>
#include <QMutex>



// Item intermédiaire entre le thread GL (readback PBO) et le thread sws (conversion RGB→YUV)
struct RgbItem {
	Frame   *frame;
	uint8_t *rgbData; // buffer av_malloc'd, libéré par SwsWorker
	int      w, h;
};

// Thread dédié à la conversion colorimétrique RGB→YUV (sws_scale), découplé du thread GL
class SwsWorker : public QThread {
public:
	SwsWorker( MQueue<RgbItem*> *pending, MQueue<Frame*> *encode );
	~SwsWorker();
	volatile bool running;
private:
	void run() override;
	MQueue<RgbItem*> *pendingRgb;
	MQueue<Frame*>   *encodeVideoFrames;
};



class Metronom : public QThread
{
	Q_OBJECT
public:
	Metronom( PlaybackBuffer *pb );
	~Metronom();
	void setRenderMode( bool b );
	void play( bool b, bool backward = false );
	bool isPlaying() { return isRunning(); }
	void changeSpeed( int s );
	void flush();
	Frame* getLastFrame();
	Frame* getAndLockLastFrame();
	void unlockLastFrame();

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

	PlaybackBuffer *playbackBuffer;
	QGLWidget *fencesContext;

	bool renderMode;
	Frame *lastFrame;
	QMutex lastFrameMutex;

signals:
	void newFrame( Frame* );
	void currentFramePts( double );
	void discardFrame( int );

	void osdMessage( const QString &text, int duration );
	void osdTimer( bool );
};

#endif // METRONOM_H
