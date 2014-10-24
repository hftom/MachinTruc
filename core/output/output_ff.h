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
	OutputFF( MQueue<Frame*> *vf, MQueue<Frame*> *af );
	~OutputFF();
	
	bool init( QString filename, Profile &prof, int vrate, bool mpeg, double end );
	void startEncode();
	bool cancel();
	
private:
	void run();
	void close();
	bool openFormat( QString filename, Profile &prof, int vrate, bool mpeg );
	bool openVideo( Profile &prof, int vrate, bool mpeg );
	bool openAudio( Profile &prof );
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
	double firstAudioPts;

	double endPTS;
	
signals:
	void showFrame( Frame* );
};

#endif // OUTPUTFF_H
