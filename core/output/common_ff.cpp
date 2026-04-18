#include <QDebug>
#include <QMutex>

#include "common_ff.h"
#include "engine/profile.h"



static FFmpegCommon globalFFmpeg;



FFmpegCommon::FFmpegCommon()
	: initDone( false )
{
}



bool FFmpegCommon::initFFmpeg()
{
	if ( !initDone ) {
		initDone = true;
		
		/*const AVCodecDescriptor *desc = NULL;
		desc = avcodec_descriptor_next( desc );
		while ( desc ) {
			qDebug() << desc->name;
			//qDebug() << "	" << desc->long_name;
			desc = avcodec_descriptor_next( desc );
		}*/
		
		/*const AVOutputFormat *format = NULL;
		format = av_oformat_next( format );
		while ( format ) {
			qDebug() << format->name << avcodec_get_name( format->video_codec ) << avcodec_get_name( format->audio_codec );
			format = av_oformat_next( format );
		}*/
		
		h264CodecNames << "default";
		hevcCodecNames << "default";
		
		void *codecIter = NULL;
		const AVCodec *codec = NULL;
		while ( (codec = av_codec_iterate( &codecIter )) ) {
			if ( codec->type == AVMEDIA_TYPE_VIDEO && av_codec_is_encoder( codec ) ) {
				QString cn = codec->name;
				if (cn.contains("264")) {
					qDebug() << codec->name;
					h264CodecNames << codec->name;
				}
				else if (cn.contains("hevc") || cn.contains("265")) {
					qDebug() << codec->name;
					hevcCodecNames << codec->name;
				}
			}
		}
	}

	return initDone;
}



AVSampleFormat FFmpegCommon::convertProfileSampleFormat( int f )
{
	if ( f == Profile::SAMPLE_FMT_32F )
		return AV_SAMPLE_FMT_FLT;

	return AV_SAMPLE_FMT_S16;
}



qint64 FFmpegCommon::convertProfileAudioLayout( int layout )
{
	if ( layout == Profile::LAYOUT_51 )
		return AV_CH_LAYOUT_5POINT1 ;

	return AV_CH_LAYOUT_STEREO;
}



FFmpegCommon* FFmpegCommon::getGlobalInstance()
{
	return &globalFFmpeg;
}
