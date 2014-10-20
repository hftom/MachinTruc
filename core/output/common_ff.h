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
}



class FFmpegCommon
{
public:
	FFmpegCommon();
	bool initFFmpeg();
	static FFmpegCommon* getGlobalInstance();
	
private:
	bool initDone;
};

#endif // COMMONFF_H