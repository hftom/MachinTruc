// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#ifndef INPUTFF_H
#define INPUTFF_H

#include <QString>
#include <QSemaphore>

#include "input.h"
#include "ffdecoder.h"



class LastDecodedFrame
{
public:
	LastDecodedFrame() : buffer(NULL), type(0), pts(0), orientation(0) {}
	~LastDecodedFrame() {
		if ( buffer )
			BufferPool::globalInstance()->releaseBuffer( buffer );
	}
	void set( Frame *f ) {
		if ( buffer )
			BufferPool::globalInstance()->releaseBuffer( buffer );
		buffer = NULL;
		if ( f ) {
			buffer = f->getBuffer();
			BufferPool::globalInstance()->useBuffer( buffer );
			profile = f->profile;
			type = f->type();
			pts = f->pts() + f->profile.getVideoFrameDuration();
			orientation = f->orientation();
		}
	}
	void get( Frame *f ) {
		if ( buffer )
			f->setSharedBuffer( buffer );
		f->setVideoFrame( (Frame::DataType)type, profile.getVideoWidth(), profile.getVideoHeight(), profile.getVideoSAR(),
						  profile.getVideoInterlaced(), profile.getVideoTopFieldFirst(), pts, profile.getVideoFrameDuration(), orientation );
		f->profile = profile;
		pts += profile.getVideoFrameDuration();
	}
	bool valid() { return buffer != NULL; }

private:
	Buffer *buffer;
	int type;
	double pts;
	Profile profile;
	int orientation;
};



class VideoResampler
{
public:
	VideoResampler()
		: repeatPTS( 0 )
	{
		reset( 40000 );
	}
	void reset( double dur ) {
		outputPts = 0;
		outputDuration = dur;
		repeat = 0;
	}
	void setRepeat( double pts, int r ) {
		repeat = r;
		repeatPTS = pts;
	}
	void duplicate( Frame *f ) {
		f->setPts( repeatPTS + outputDuration );
		if ( (repeat - 1) > 0 )
			setRepeat( f->pts(), repeat - 1 );
		else
			setRepeat( 0, 0 );
		outputPts += outputDuration;
	}

	double outputPts;
	double outputDuration;

	int repeat;
	double repeatPTS;
};



class InputFF : public InputBase
{
	Q_OBJECT
public:
	InputFF();
	~InputFF();
	bool open( QString fn );
	void openSeekPlay( QString fn, double p );
	double seekTo( double p );
	void play( bool b );
	Frame *getVideoFrame();
	Frame *getAudioFrame( int nSamples );

	bool probe( QString fn, Profile *prof );

	void setProfile( const Profile &in, const Profile &out ) {
		InputBase::setProfile( in, out );
		decoder->setProfile( in, out );
	}

protected:
	void run();

private:
	void flush();
	void resample( Frame *f );

	FFDecoder *decoder;

	MQueue<Frame*> audioFrames;
	MQueue<Frame*> freeAudioFrames;

	MQueue<Frame*> videoFrames;
	MQueue<Frame*> freeVideoFrames;

	QSemaphore *semaphore;
	bool running;

	double seekAndPlayPTS;
	QString seekAndPlayPath;
	bool seekAndPlay;

	LastDecodedFrame lastFrame;
	VideoResampler videoResampler;
};

#endif // INPUTFF_H
