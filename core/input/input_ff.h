// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#ifndef INPUTFF_H
#define INPUTFF_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

#include <QString>
#include <QQueue>
#include <QWaitCondition>

#include "input/input.h"



class AudioChunk
{
public:
	AudioChunk() {
		bytes = NULL;
		bufSize = 0;
		bufOffset = 0;
		available = 0;
		pts = 0;
	}
	~AudioChunk() {
		if ( bytes )
			free( bytes );
	}

	double pts;
	uint8_t *bytes;
	int bufSize, bufOffset; // in bytes
	int available; // in samples
};



class AudioRingBuffer
{
public:
	AudioRingBuffer();
	~AudioRingBuffer();
	void reset();
	bool readable( int nSamples );
	bool writable();
	void read( uint8_t *dst, int nSamples );
	double readPts();
	int read( uint8_t *dst ); // reads all available samples
	//uint8_t* write( int nSamples );
	uint8_t* write( int writtenSamples, int moreSamples );
	void writeDone( double pts, int nSamples, int samplesOffset=0 ); // MUST be called right after last "write"
	void setBytesPerChannel( int n );
	int getBytesPerChannel();
	void setChannels( int c );
	int getChannels();
	void setSampleRate( int r );
	int getSampleRate();
	int getBytesPerSample();

private:
	AudioChunk *chunks;
	int reader, writer;
	int size;
	int bytesPerSample;
	int bytesPerChannel;
	int sampleRate;
	int channels;

	QMutex mutex;
};



class AudioPacket
{
public:
	AudioPacket() {
		free();
	}
	~AudioPacket() {}
	void set( AVPacket *p ) {
		packet = p;
		orgData = p->data;
		orgSize = p->size;
		pts = AV_NOPTS_VALUE;
	}
	void reset() {
		if ( packet ) {
			packet->data = orgData;
			packet->size = orgSize;
		}
	}
	void free() {
		packet = NULL;
	}

	AVPacket *packet;
	uint8_t *orgData;
	int orgSize;
	double pts;
};



class VideoResampler
{
public:
	VideoResampler() {
		reset( 40000 );
	}
	~VideoResampler() {}
	void reset( double dur ) {
		outputPts = 0;
		outputDuration = dur;
		repeat = 0;
		repeatFrame = NULL;
	}
	void setRepeat( Frame *f, int r ) {
		repeat = r;
		repeatFrame = f;
		if ( f ) {
			//printf("repeat frame = %d\n", r);
			profile = f->profile;
			repeatType = f->type();
			repeatPTS = f->pts();
		}
	}

	void duplicate( Frame *f ) {
		f->setVideoFrame( (Frame::DataType)repeatType, profile.getVideoWidth(), profile.getVideoHeight(), profile.getVideoAspectRatio(),
						  profile.getVideoInterlaced(), profile.getVideoTopFieldFirst(), repeatPTS + outputDuration, profile.getVideoFrameDuration() );
		f->profile = profile;
		int num = (repeatType == Frame::YUV420P) ? 3 : 4;
		memcpy( f->data(), repeatFrame->data(), profile.getVideoWidth() * profile.getVideoHeight() * num / 2 );
		if ( (repeat - 1) > 0 )
			setRepeat( f, repeat - 1 );
		else
			setRepeat( NULL, 0 );
		outputPts += outputDuration;
	}

	double outputPts;
	double outputDuration;

	int repeat;
	Frame *repeatFrame;
	int repeatType;
	double repeatPTS;
	Profile profile;
};



class InputFF : public InputBase
{
	Q_OBJECT
public:
	InputFF();
	~InputFF();
	bool open( QString fn );
	void close();
	void flush();
	void seekFast( float percent );
	void seekNext();
	double seekTo( double p );
	void openSeekPlay( QString fn, double p );
	void play( bool b );
	Frame *getVideoFrame();
	Frame *getAudioFrame( int nSamples );

	bool probe( QString fn, Profile *prof );

protected:
	void run();

private:
	bool ffOpen( QString fn );
	bool getPacket();
	void freePacket( AVPacket *packet );
	bool decodeVideo( Frame *f );
	bool makeFrame( Frame *f, double ratio, double pts, double dur );
	bool decodeAudio( int sync=0, double *pts=NULL );
	void freeCurrentAudioPacket();
	void shiftCurrentAudioPacket( int len );
	void shiftCurrentAudioPacketPts( double pts );
	void resetAudioResampler( bool resetArb = true );
	AVSampleFormat convertProfileSampleFormat( int f );
	qint64 convertProfileAudioLayout( int nChannels );
	bool seek( double t );
	bool seekDecodeNext( Frame *f );

	void wakeUp();

	AVFormatContext *formatCtx;
	AVCodecContext *videoCodecCtx;
	AVCodecContext *audioCodecCtx;
	SwrContext *swr;
	int lastChannels;
	int64_t lastLayout;
	AVCodec *videoCodec;
	AVCodec *audioCodec;
	int videoStream;
	int audioStream;
	AVFrame *videoAvframe;
	AVFrame *audioAvframe;

	double duration;
	double startTime;

	double seekAndPlayPTS;
	bool seekAndPlay;
	QString seekAndPlayPath;

	AudioRingBuffer *arb;
	AudioPacket currentAudioPacket;

	VideoResampler videoResampler;

	QQueue<AVPacket*> audioPackets, videoPackets;

	MQueue<Frame*> audioFrames;
	MQueue<Frame*> freeAudioFrames;

	MQueue<Frame*> videoFrames;
	MQueue<Frame*> freeVideoFrames;

	enum EofMode{ EofPacket=1, EofAudioPacket=2, EofVideoPacket=4, EofAudio=8, EofVideo=16 };
	int endOfFile;

	bool running, oneShot;
	QMutex mutex;
	QMutex waitMutex, runningMutex;
	QWaitCondition waitCond;
};

#endif //INPUTFF_H
