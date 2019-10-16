// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#ifndef FRAME_H
#define FRAME_H

#include <stdint.h>

#include <QList>
#include <QMutex>
#include <QDebug>
#include <QStringList>
#include <QSharedPointer>
#include <QRectF>

#include "bufferpool.h"
#include "profile.h"
#include "glresource.h"
#include "afx/audiofilter.h"
#include "vfx/glfilter.h"



class ProjectSample;
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
	enum DataType{ NONE, YUV420P, YUV422P, YUV444P, RGBA, RGB, GLSL, GLTEXTURE, LAST };

	Frame( MQueue<Frame*> *origin = NULL );
	~Frame();
	void setType( int t ) { pType = t; }
	int type() { return pType; }
	// Push the frame back in origin MQueue, release the texture and release tracks.
	void release();

	void setVideoFrame( DataType t, int w, int h, double sar, bool il, bool tff, double p, double d, int rot = 0, int size = 0, int bits = 8 );
	void setVideoFrame( Frame *src );
	void setFBO( FBO *f );
	FBO* fbo() { return fb; }
	void setPBO( PBO *p );
	PBO* pbo() { return pb; }
	void setFence( FENCE *f );
	FENCE* fence() { return glfence; }

	void setAudioFrame( int c, int r, int bpc, int samples, double p );
	int audioSamples() { return pAudioSamples; }
	void setAudioSamples( int n ) { pAudioSamples = n; }
	bool audioReversed;

	// PTS in microsecond
	void setPts( double p ) { pPTS = p; }
	double pts() { return pPTS; }

	int orientation() { return pOrientation; }

	int bitDepth;

	// memory management indicator. See input.h
	quint32 mmi;
	QString mmiProvider;
	// video or audio data
	Buffer* getBuffer() { return buffer; }
	void setSharedBuffer( Buffer *b );
	uint8_t *data() { return (buffer ? buffer->data() : NULL); }

	// The list of input Frame used to compose this output one.
	ProjectSample *sample;
	// indicate that this frame should not be pushed in playbackBuffer
	bool isDuplicate;

	// frame profile
	Profile profile;

	// composer helpers
	int glWidth, glHeight;
	double glSAR;
	int glOVD;
	QRectF glOVDRect;
	QList<FilterTransform> glOVDTransformList;
	bool paddingAuto, resizeAuto, sarAuto;

private:
	int pType;
	FBO *fb;
	PBO *pb;
	FENCE *glfence;
	int pAudioSamples;
	double pPTS;
	int pOrientation;

	Buffer *buffer;

	MQueue<Frame*> *originQueue;
};



class TransitionSample
{
public:
	TransitionSample() : frame(NULL) {}
	void clear( bool releaseFrame ) {
		videoFilters.clear();
		audioFilters.clear();
		if ( frame && releaseFrame ) {
			delete frame;
			frame = NULL;
		}
		videoTransitionFilter.clear();
		audioTransitionFilter.clear();
	}

	Frame *frame;
	QList< QSharedPointer<GLFilter> > videoFilters;
	QList< QSharedPointer<AudioFilter> > audioFilters;

	QSharedPointer<GLFilter> videoTransitionFilter;
	QSharedPointer<AudioFilter> audioTransitionFilter;
};



class FrameSample
{
public:
	FrameSample() : frame(NULL) {}
	void clear( bool releaseFrame = true ) {
		videoFilters.clear();
		audioFilters.clear();
		if ( frame && releaseFrame ) {
			delete frame;
			frame = NULL;
		}
		transitionFrame.clear( releaseFrame );
	}

	Frame *frame;
	TransitionSample transitionFrame;
	QList< QSharedPointer<GLFilter> > videoFilters;
	QList< QSharedPointer<AudioFilter> > audioFilters;
};



class ProjectSample
{
public:
	ProjectSample() {}
	// for video only
	void copyVideoSample( ProjectSample *src ) {
		clear();
		for ( int i = 0; i < src->frames.count(); ++i ) {
			FrameSample *sfs = src->frames.at(i);
			FrameSample *fs = new FrameSample();
			frames.append( fs );
			if ( sfs->frame ) {
				fs->frame = new Frame();
				fs->frame->setVideoFrame( sfs->frame );
				fs->frame->setType( sfs->frame->type() );
				if ( sfs->frame->getBuffer() )
					fs->frame->setSharedBuffer( sfs->frame->getBuffer() );
				fs->videoFilters = sfs->videoFilters;
				if ( sfs->transitionFrame.frame ) {
					fs->transitionFrame.frame = new Frame();
					fs->transitionFrame.frame->setVideoFrame( sfs->transitionFrame.frame );
					fs->transitionFrame.frame->setType( sfs->transitionFrame.frame->type() );
					if ( sfs->transitionFrame.frame->getBuffer() )
						fs->transitionFrame.frame->setSharedBuffer( sfs->transitionFrame.frame->getBuffer() );
					fs->transitionFrame.videoFilters = sfs->transitionFrame.videoFilters;
					fs->transitionFrame.videoTransitionFilter = sfs->transitionFrame.videoTransitionFilter;
				}
			}
		}
	}
	~ProjectSample() {
		clear();
	}
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
