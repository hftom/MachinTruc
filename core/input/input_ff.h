// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#ifndef INPUTFF_H
#define INPUTFF_H

#include <QString>
#include <QSemaphore>

#include "input.h"
#include "ffdecoder.h"



class AudioFrameList
{
public:
	AudioFrameList() : maxSamples( 11520 ),
		bytesPerSample( 4 ),
		sampleRate( 48000 ) {
	}
	~AudioFrameList() {
		while ( !list.isEmpty() )
			delete list.takeFirst();
	}
	void reset( Profile p ) {
		QMutexLocker ml( &mutex );
		while ( !list.isEmpty() )
			delete list.takeFirst();
		bytesPerSample = Profile::bytesPerChannel( &p ) * p.getAudioChannels();
		sampleRate = p.getAudioSampleRate();
		maxSamples = (((double)sampleRate / p.getVideoFrameRate()) + 1) * (NUMINPUTFRAMES + 1);
	}
	bool readable( int nSamples ) {
		QMutexLocker ml( &mutex );
		int ns = 0;
		for ( int i = 0; i < list.count(); ++i ) {
			ns += list[i]->available;
			if ( ns >= nSamples )
				return true;
		}
		return false;
	}
	bool writable() {
		QMutexLocker ml( &mutex );
		int ns = 0;
		for ( int i = 0; i < list.count(); ++i ) {
			ns += list[i]->available;
		}
		return ns < maxSamples;
	}
	void read( uint8_t *dst, int nSamples ) {
		QMutexLocker ml( &mutex );
		uint8_t *d = dst;
		while ( nSamples > 0 ) {
			bool shift = false;
			AudioFrame *frame = list.first();
			int ns = frame->available;
			if ( ns > nSamples ) {
				ns = nSamples;
				shift = true;
			}
			int blen = ns * bytesPerSample;
			memcpy( d, frame->buffer->data() + frame->bufOffset, blen );
			if ( shift ) {
				frame->bufOffset += blen;
				frame->available -= ns;
				frame->bufPts += (double)ns * MICROSECOND / (double)sampleRate;
				return;
			}
			delete list.takeFirst();
			nSamples -= ns;
			d += blen;
		}
	}
	double readPts() {
		QMutexLocker ml( &mutex );
		if ( list.isEmpty() )
			return 0;
		return list.first()->bufPts;
	}
	int read( uint8_t *dst ) { // reads all available samples
		QMutexLocker ml( &mutex );
		uint8_t *d = dst;
		int nSamples = 0;
		while ( !list.isEmpty() ) {
			AudioFrame *frame = list.takeFirst();
			int ns = frame->available;
			int blen = ns * bytesPerSample;
			memcpy( d, frame->buffer->data() + frame->bufOffset, blen );
			nSamples += ns;
			d += blen;
			delete frame;
		}
		return nSamples;
	}
	int getBytesPerSample() {
		return bytesPerSample;
	}
	void append( AudioFrame *af ) {
		QMutexLocker ml( &mutex );
		list.append( af );
	}
	int count() {
		QMutexLocker ml( &mutex );
		return list.count();
	}

private:
	QList<AudioFrame*> list;
	int maxSamples;

	int bytesPerSample;
	int sampleRate;

	QMutex mutex;
};



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
			pts = f->pts();
			orientation = f->orientation();
		}
	}
	void get( Frame *f, bool backward = false ) {
		if ( buffer )
			f->setSharedBuffer( buffer );
		if ( backward )
			pts -= profile.getVideoFrameDuration();
		else
			pts += profile.getVideoFrameDuration();
		f->setVideoFrame( (Frame::DataType)type, profile.getVideoWidth(), profile.getVideoHeight(), profile.getVideoSAR(),
						  profile.getVideoInterlaced(), profile.getVideoTopFieldFirst(), pts, profile.getVideoFrameDuration(), orientation );
		f->profile = profile;
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
	void duplicate( Frame *f, bool backward = false ) {
		if ( backward )
			f->setPts( repeatPTS - outputDuration );
		else
			f->setPts( repeatPTS + outputDuration );
		if ( (repeat - 1) > 0 )
			setRepeat( f->pts(), repeat - 1 );
		else
			setRepeat( 0, 0 );
		if ( backward )
			outputPts -= outputDuration;
		else
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
	void openSeekPlay( QString fn, double p, bool backward = false );
	double seekTo( double p );
	void play( bool b );
	Frame *getVideoFrame();
	Frame *getAudioFrame( int nSamples );

	bool probe( QString fn, Profile *prof );

	void setProfile( const Profile &in, const Profile &out ) {
		InputBase::setProfile( in, out );
		decoder->setProfile( in, out );
		audioFrameList.reset( out );
	}

protected:
	void run();

private:
	void flush();
	void resample( Frame *f );
	void resampleBackward( Frame *f );

	void runForward();
	void runBackward();

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

	QList<Frame*> backwardVideoFrames;
	MQueue<Frame*> reorderedVideoFrames;
	QList<AudioFrame*> backwardAudioFrames;
	int backwardAudioSamples;
	bool playBackward;
	double backwardPts, backwardStartPts;
	bool backwardEof;

	LastDecodedFrame lastFrame;
	VideoResampler videoResampler;

	AudioFrameList audioFrameList;

	bool eofVideo, eofAudio;
};

#endif // INPUTFF_H
