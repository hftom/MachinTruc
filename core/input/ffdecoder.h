// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#ifndef FFDECODER_H
#define FFDECODER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "output/common_ff.h"

#include <QString>
#include <QQueue>

#include <QMutex>
#include "engine/frame.h"



class AudioFrame
{
public:
	explicit AudioFrame( int sampleSize, int rate )
		: bufPts( 0 ),
		buffer( NULL ),
		bufSize( 0 ),
		bufOffset( 0 ),
		available( 0 ),
		bytesPerSample( sampleSize ),
		sampleRate( rate )
	{
	}
	~AudioFrame() {
		if ( buffer )
			BufferPool::globalInstance()->releaseBuffer( buffer );
	}
	uint8_t* write( int writtenSamples, int moreSamples ) {
		int size = (writtenSamples + moreSamples) * bytesPerSample;
		if ( size < 1 )
			return NULL;
		if ( !buffer ) {
			bufSize = size;
			buffer = BufferPool::globalInstance()->getBuffer( bufSize );
		}
		else if ( bufSize < size ) {
			bufSize = size;
			buffer = BufferPool::globalInstance()->enlargeBuffer( buffer, bufSize );
		}

		return buffer->data() + (writtenSamples * bytesPerSample);
	}
	void writeDone( double pts, int nSamples, int samplesOffset = 0 ) {
		bufPts = pts;
		available = nSamples - samplesOffset;
		bufOffset = samplesOffset * bytesPerSample;
	}

	double bufPts;
	Buffer *buffer;
	int bufSize, bufOffset; // in bytes
	int available; // in samples
	int bytesPerSample;
	int sampleRate;
};



class AudioPacket
{
public:
	AudioPacket() : orgData(NULL), orgSize(0), pts(0) {
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



class Yadif
{
public:
	Yadif();
	~Yadif();
	bool reset( bool sendFields, int videoStreamIndex, AVFormatContext *fmtCtx, AVCodecContext *codecCtx );
	bool pushFrame( AVFrame *f, double pts, double duration, double ratio );
	AVFrame* pullFrame( double &pts, double &duration, double &ratio );

	bool eof;

private:
	bool twice;
	AVFrame *filterFrame;
	AVFilterContext *bufferSinkCtx;
	AVFilterContext *bufferSrcCtx;
	AVFilterGraph *filterGraph;
	QQueue<double> ptsQueue;
	QQueue<double> durationQueue;
	double ar;
};



class FFDecoder
{
private:
	friend class InputFF;

	enum YadifMode{ NoYadif=0, Yadif1X=1, Yadif2X=2 };
	FFDecoder();
	~FFDecoder();
	bool open( QString fn );
	bool seekTo( double p, Frame *f, AudioFrame *af );
	bool probe( QString fn, Profile *prof );
	bool decodeVideo( Frame *f );
	bool decodeAudio( AudioFrame *f, int sync=0, double *pts=NULL );
	void setProfile( const Profile &in, const Profile &out ) {
		inProfile = in;
		outProfile = out;
		if ( inProfile.getVideoInterlaced() ) {
			if ( outProfile.getVideoFrameRate() > inProfile.getVideoFrameRate() )
				doYadif = Yadif2X;
			else
				doYadif = Yadif1X;
		}
		else
			doYadif = NoYadif;
	}

	void close();
	void flush();
	bool ffOpen( QString fn );
	bool getPacket();
	void freePacket( AVPacket *packet );
	bool makeFrame( Frame *f, AVFrame *avFrame, double ratio, double pts, double dur );
	void copyYUVPlanar(uint8_t *buf, AVFrame *avFrame, int h[3], int w[3], int bits);
	void freeCurrentAudioPacket();
	void shiftCurrentAudioPacket( int len );
	void shiftCurrentAudioPacketPts( double pts );
	void resetAudioResampler();
	bool seek( double t );
	bool seekDecodeNext( Frame *f );

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
	Yadif yadif;
	int doYadif;

	int orientation;

	double duration;
	double startTime;

	AudioPacket currentAudioPacket;

	QQueue<AVPacket*> audioPackets, videoPackets;

	enum EofMode{ EofPacket=1, EofAudioPacket=2, EofVideoPacket=4, EofAudio=8, EofVideo=16 };
	int endOfFile;

	bool haveAudio, haveVideo;
	Profile inProfile, outProfile;
};

#endif // FFDECODER_H
