#ifndef COMPOSER_H
#define COMPOSER_H

#include "movitchain.h"
#include "vfx/movitbackground.h"
#include "filtercollection.h"

#include <QGLWidget>
#include <QThread>

#include "engine/sampler.h"



class Composer : public QThread
{
	Q_OBJECT
public:
	Composer( Sampler *samp, PlaybackBuffer *pb );
	~Composer();

	void play( bool b, bool backward = false );
	void seeking();
	void runOneShot();
	void updateFrame( Frame *dst );
	bool isPlaying();

public slots:
	void setSharedContext( QGLWidget *shared );
	void discardFrame( int );

private:
	void run();
	
	int process( Frame **frame );
	Frame* getNextFrame( Frame *dst, int &track );
	void waitFence();
	bool renderVideoFrame( Frame *dst );
	void movitFrameDescriptor( QString prefix, Frame *f, QList< QSharedPointer<GLFilter> > *filters, QStringList &desc, Profile *projectProfile );
	Effect* movitFrameBuild( Frame *f, QList< QSharedPointer<GLFilter> > *filters, MovitBranch **newBranch );
	void movitRender( Frame *dst, bool update = false );
	bool getNextAudioFrame( Frame *dst, int &track );
	Buffer* processAudioFrame( FrameSample *sample, int nsamples, int bitsPerSample, Profile *profile );
	bool renderAudioFrame( Frame *dst, int nSamples );

	bool playBackward;
	bool running, playing;
	int renderMode;
	bool oneShot;
	int skipFrame;

	GLuint mask_texture;
	
	QGLWidget *hiddenContext;
	GLResource gl;
	FENCE *composerFence;

	MovitBackground movitBackground;
	MovitChain movitChain;
	ResourcePool *movitPool;

	Sampler *sampler;
	PlaybackBuffer *playbackBuffer;
	double audioSampleDelta;

signals:
	void newFrame( Frame* );
	void paused( bool );

};

#endif // COMPOSER_H
