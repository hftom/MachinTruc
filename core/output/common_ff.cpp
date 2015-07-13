#include <QDebug>
#include <QMutex>

#include "common_ff.h"



static FFmpegCommon globalFFmpeg;

static int lockManager( void **mutex, enum AVLockOp op)
{
	QMutex **m = (QMutex**)mutex;
	switch ( op ) {
		case AV_LOCK_CREATE:
			(*m) = new QMutex();
			break;
		case AV_LOCK_OBTAIN:
			(*m)->lock();
			break;
		case AV_LOCK_RELEASE:
			(*m)->unlock();
			break;
		case AV_LOCK_DESTROY:
			delete (*m);
			break;
	}
	return 0;
}



FFmpegCommon::FFmpegCommon()
	: initDone( false )
{
}



bool FFmpegCommon::initFFmpeg()
{
	if ( !initDone ) {
		av_lockmgr_register( lockManager );
		avcodec_register_all();
		av_register_all();
		avfilter_register_all();
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
	}

	return initDone;
}



FFmpegCommon* FFmpegCommon::getGlobalInstance()
{
	return &globalFFmpeg;
}
