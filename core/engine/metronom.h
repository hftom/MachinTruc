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
    void play( bool b );
    Frame* getLastFrame();
    void flush();

    static void readData( Frame **data, double time, void *userdata );

	void setSharedContext( QGLWidget *shared );

    MQueue<Frame*> videoFrames;
    MQueue<Frame*> freeVideoFrames;
    MQueue<Frame*> audioFrames;
    MQueue<Frame*> freeAudioFrames;

public slots:
    void setLastFrame( Frame *f );

protected:
    void run();

private:
    bool running;
    double sclock, videoLate;
    QMutex clockMutex;
    AudioOutSDL ao;

	QGLWidget *fencesContext;

    Frame *lastFrame;
    QMutex lastFrameMutex;

signals:
    void newFrame( Frame* );
	void currentFramePts( double );
	void discardFrame();

};

#endif // METRONOM_H
