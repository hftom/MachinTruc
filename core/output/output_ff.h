#ifndef OUTPUTFF_H
#define OUTPUTFF_H

#include <QThread>
#include <QString>

#include "engine/frame.h"
#include "common_ff.h"



class OutputFF : public QThread
{
	Q_OBJECT
public:
	enum videoCodec{VCODEC_HEVC, VCODEC_H264, VCODEC_MPEG2};

	OutputFF( MQueue<Frame*> *vf, MQueue<Frame*> *af );
	~OutputFF();
	
	bool init( QString filename, Profile &prof, int vrate, int vcodec, double end );
	void startEncode( bool show = true );
	bool cancel();
	
private:
	void run();
	void close();
	bool openFormat( QString filename, Profile &prof, int vrate, int vcodec );
	bool openVideo( Profile &prof, int vrate, int vcodec );
	bool openAudio( Profile &prof, int vcodec );
	bool encodeVideo( Frame *f, int nFrame );
	bool encodeAudio( Frame *f, int nFrame );
	
	MQueue<Frame*> *audioFrames;
	MQueue<Frame*> *videoFrames;
	bool running;
	
	AVFormatContext *formatCtx;
	
	AVStream *videoStream;
	AVFrame *videoFrame;
	
	AVStream *audioStream;
	AVFrame *audioFrame;
	uint8_t *audioSamples;
	int audioSamplesSize;
	uint8_t *audioBuffer;
	int audioBufferLen;
	int totalSamples;
	int audioCodecFrameSize;
	SwrContext *swr;

	double endPTS;
	
	bool showFrameProgress;
	
signals:
	void showFrame( Frame* );
};

#endif // OUTPUTFF_H
