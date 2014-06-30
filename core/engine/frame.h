// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>

#include <QList>
#include <QMutex>
#include <QDebug>
#include <QStringList>

#include "engine/profile.h"
#include "engine/glresource.h"

#define NUMINPUTFRAMES 5
#define NUMOUTPUTFRAMES 4



class ProjectSample;
class GLComposition;
class GLFilter;
class AudioFilter;



// A thread safe queue.
template <class T>
class MQueue : public QList<T>
{
public:
    MQueue() {}
    ~MQueue() {}
    void enqueue( const T &t ) {
        mutex.lock();
        QList<T>::append(t);
        mutex.unlock();
    }
    T dequeue() {
        mutex.lock();
        T t = NULL;
        if ( !QList<T>::isEmpty() )
            t = QList<T>::takeFirst();
        mutex.unlock();
        return t;
    }
    bool queueEmpty() {
        mutex.lock();
        bool b = QList<T>::isEmpty();
        mutex.unlock();
        return b;
    }

private:
    QMutex mutex;
};



class Frame
{
public:
    enum DataType{ NONE, YUV420P, YUV422P, RGBA, RGB, GLSL, GLTEXTURE };

    Frame( MQueue<Frame*> *origin, bool makeSample=false );
    ~Frame();
	void setType( int t ) { pType = t; }
    int type() { return pType; }
    // Push the frame back in origin MQueue, release the texture and release tracks.
    void release();

	void setVideoFrame( DataType t, int w, int h, double sar, bool il, bool tff, double p, double d );
    void setVideoFrame( Frame *src );
    void setTexture( TEXTURE *t );
    TEXTURE* texture() { return tex; }
    void setFence( FENCE *f );
    FENCE* fence() { return glfence; }

    void setAudioFrame( int c, int r, int bpc, int samples, double p );
    int audioSamples() { return pAudioSamples; }

    // PTS in microsecond
    void setPts( double p ) { pPTS = p; }
    double pts() { return pPTS; }

	// video or audio data
    uint8_t *data() { return buffer; }

    // The list of input Frame used to compose this output one.
    ProjectSample *sample;

	// frame profile
	Profile profile;

	// composer helpers
	int glWidth, glHeight;
	double glSAR;
	bool paddingAuto, resizeAuto;

private:
    void resizeBuffer( int s );

	int pType;
	TEXTURE *tex;
	FENCE *glfence;
    int pAudioSamples;
    double pPTS;

	uint8_t *buffer;
    int bufferSize;

    MQueue<Frame*> *originQueue;
};



class FrameSample
{
public:
    FrameSample() {
        frame = NULL;
		composition = NULL;
    }
    ~FrameSample() {}
    void clear( bool releaseFrame = true );

    Frame* frame;
    GLComposition* composition;
    QList<GLFilter*> videoFilters;
    QList<AudioFilter*> audioFilters;
};



class ProjectSample
{
public:
    ProjectSample() {}
    ~ProjectSample() {}
    void clear() {
        while ( !frames.isEmpty() ) {
            FrameSample *f = frames.takeFirst();
            f->clear();
            delete f;
        }
    }

    QList<FrameSample*> frames;
};

#endif // FRAME_H
