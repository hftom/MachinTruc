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
	
	bool init( QString filename, Profile &prof, int vrate, double end );
	void startEncode();
	bool cancel();
	
private:
	void run();
	void close();
	bool openVideo( QString filename, Profile &prof, int vrate );
	bool openAudio( QString filename, Profile &prof );
	bool encodeVideo( Frame *f, int nFrame );
	bool encodeAudio( Frame *f, int nFrame );
	
	MQueue<Frame*> *audioFrames;
	MQueue<Frame*> *videoFrames;
	bool running;
	AVCodecContext *videoCodecCtx;
	AVFrame *videoFrame;
	FILE *videoFile;
	
	AVCodecContext *audioCodecCtx;
	AVFrame *audioFrame;
	uint8_t *audioSamples;
	int audioSamplesSize;
	uint8_t *audioBuffer;
	int audioBufferLen;
	FILE *audioFile;

	double endPTS;
	
signals:
	void showFrame( Frame* );
};

#endif // OUTPUTFF_H
