#ifndef COMPOSER_H
#define COMPOSER_H

#include "movitchain.h"
#include "vfx/movitbackground.h"
#include "filtercollection.h"

#include <QGLWidget>
#include <QThread>

#include "engine/sampler.h"



class ItcMsg
{
public:
	enum MsgType{ RENDERSTOP, RENDERPLAY, RENDERSETPLAYBACKBUFFER, RENDERFRAMEBYFRAME, RENDERFRAMEBYFRAMESETPLAYBACKBUFFER, RENDERSKIPBY, RENDERSEEK, RENDERUPDATE };
	ItcMsg()
		: msgType( RENDERSTOP ) {}
	ItcMsg( int type )
		: msgType( type ) {}
	ItcMsg( double p, bool b, bool s )
		: msgType( RENDERSEEK ), pts( p ), backward( b ), seek( s ) {}
	ItcMsg( bool bw )
		: msgType( RENDERSETPLAYBACKBUFFER ), backward( bw ) {}
	ItcMsg( int type, int s )
		: msgType( type ), step( s ) {}
	ItcMsg( int type, bool bw )
		: msgType( type ), backward( bw ) {}
		
	int msgType;
	double pts;
	bool backward, seek;
	int step;
};



class Composer : public QThread
{
	Q_OBJECT
public:
	Composer( Sampler *samp, PlaybackBuffer *pb );
	~Composer();

	void play( bool b, bool backward = false );
	void setPlaybackBuffer( bool backward );
	void seekTo( double p, bool backward = false, bool seek = true );
	void frameByFrame();
	void frameByFrameSetPlaybackBuffer( bool backward );
	void skipBy( int step );
	void updateFrame();
	bool isPlaying();
	
	void setOutputResize( QSize size ) { outputResize = size; }

public slots:
	void setSharedContext( QGLWidget *shared );
	void discardFrame( int );

private:
	void run();
	void runOneShot( Frame *f );
	
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
	bool oneShot;
	int skipFrame;
	ItcMsg lastMsg;
	QList<ItcMsg> itcMsgList;
	QMutex itcMutex;

	GLuint mask_texture;
	
	QGLWidget *hiddenContext;
	GLResource gl;
	FENCE *composerFence;
	GLResize resizeFilter;
	GLPadding paddingFilter;

	MovitBackground movitBackground;
	MovitChain movitChain;
	ResourcePool *movitPool;

	Sampler *sampler;
	PlaybackBuffer *playbackBuffer;
	double audioSampleDelta;
	
	QSize outputResize;

signals:
	void newFrame( Frame* );
	void paused( bool );
	void flushMetronom();

};

#endif // COMPOSER_H
