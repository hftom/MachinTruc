#ifndef COMMONFF_H
#define COMMONFF_H

extern "C" {
#include <libavcodec/avcodec.h>

#include <libavformat/avformat.h>

#include <libswresample/swresample.h>
	
#include <libavutil/imgutils.h>
#include <libavutil/dict.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
//#include <libavutil/parseutils.h>

#include <libswscale/swscale.h>
	
#include <libavfilter/avfilter.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

#include <QStringList>



const int NCFR = 8;
const double CommonFrameRates[NCFR][2] = {
	{ 24000., 1001. },
	{ 24., 1. },
	{ 25., 1. },
	{ 30000., 1001. },
	{ 30., 1. },
	{ 50., 1. },
	{ 60000., 1001. },
	{ 60., 1. },
};



class FFmpegCommon
{
public:
	FFmpegCommon();
	bool initFFmpeg();
	static FFmpegCommon* getGlobalInstance();
	
	QStringList getH264Codecs() {
		return h264CodecNames;
	}
	QStringList getHevcCodecs() {
		return hevcCodecNames;
	}
	
	AVSampleFormat convertProfileSampleFormat( int f );
	qint64 convertProfileAudioLayout( int layout );
	
private:
	bool initDone;
	QStringList hevcCodecNames;
	QStringList h264CodecNames;
};

#endif // COMMONFF_H
