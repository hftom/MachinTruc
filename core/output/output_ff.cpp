#include <QDebug>
#include <QTime>

#include "output_ff.h"



OutputFF::OutputFF( MQueue<Frame*> *vf, MQueue<Frame*> *af )
	: audioFrames( af ),
	videoFrames( vf ),
	running( false ),
	videoCodecCtx( NULL ),
	videoFrame( NULL ),
	videoFile( NULL ),
	audioCodecCtx( NULL ),
	audioFrame( NULL ),
	audioSamples( NULL ),
	audioSamplesSize( 0 ),
	audioBuffer( NULL ),
	audioBufferLen( 0 ),
	audioFile( NULL ),
	endPTS( 0 )
{
	FFmpegCommon::getGlobalInstance()->initFFmpeg();
}



OutputFF::~OutputFF()
{
	close();
}



void OutputFF::close()
{	
	if ( videoFile ) {
		fclose( videoFile );
		videoFile = NULL;
	}
	if ( videoFrame ) {
		av_freep( &videoFrame->data[0] );
		//av_frame_free( &videoFrame );
		av_free( videoFrame );
		videoFrame = NULL;
	}
	if ( videoCodecCtx ) {
		avcodec_close( videoCodecCtx );
		videoCodecCtx = NULL;
	}
	
	if ( audioFile ) {
		fclose( audioFile );
		audioFile = NULL;
	}
	if ( audioSamples ) {
		av_freep( &audioSamples );
	}
	if ( audioBuffer ) {
		av_freep( &audioBuffer );
	}
	if ( audioFrame ) {
		//av_frame_free( &audioFrame );
		av_free( audioFrame );
		audioFrame = NULL;
	}
	if ( audioCodecCtx ) {
		avcodec_close( audioCodecCtx );
		audioCodecCtx = NULL;
	}
}



bool OutputFF::openVideo( QString filename, Profile &prof, int vrate )
{
	AVCodec *codec;

	AVCodecID codec_id = AV_CODEC_ID_H264;
	if ( filename.endsWith( ".m2v" ) )
		codec_id = AV_CODEC_ID_MPEG2VIDEO;
	
	videoFile = fopen( filename.toLatin1().data(), "wb" );
	if ( !videoFile ) {
		qDebug() << "Could not open" << filename;
		return false;
	}

	// find the video encoder
	codec = avcodec_find_encoder( codec_id );
	if ( !codec ) {
		qDebug() << "Video encoder not found.";
		return false;
	}

	videoCodecCtx = avcodec_alloc_context3( codec );
	if ( !videoCodecCtx ) {
		qDebug() << "Could not allocate video codec context.";
		return false;
	}

	/* put sample parameters */
	videoCodecCtx->bit_rate = vrate * 1000000;
	/* resolution must be a multiple of two */
	videoCodecCtx->width = prof.getVideoWidth();
	videoCodecCtx->height = prof.getVideoHeight();
	/* frames per second */
	videoCodecCtx->time_base = (AVRational){1,25};
	double fps = prof.getVideoFrameRate();
	for ( int i = 0; i < NCFR; ++i ) {
		double cfps = CommonFrameRates[i][0] / CommonFrameRates[i][1];
		if ( qAbs( fps - cfps ) < 1e-3 ) {
			videoCodecCtx->time_base = (AVRational){ (int)CommonFrameRates[i][1], (int)CommonFrameRates[i][0] };
			break;
		}
	}
	/*
	* emit one intra frame every ten frames
	* check frame pict_type before passing frame
	* to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	* then gop_size is ignored and the output of encoder
	* will always be I frame irrespective to gop_size
	*/
	videoCodecCtx->gop_size = 12;
	videoCodecCtx->max_b_frames = 2;
	videoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

	//if ( codec_id == AV_CODEC_ID_H264 )
		//av_opt_set( videoCodecCtx->priv_data, "preset", "veryslow", 0 );

	/* open it */
	if ( avcodec_open2( videoCodecCtx, codec, NULL ) < 0 ) {
		qDebug() << "Could not open video codec.";
		return false;
	}

	videoFrame = av_frame_alloc();
	if ( !videoFrame ) {
		qDebug() << "Could not allocate video frame.";
		return false;
	}
	videoFrame->format = videoCodecCtx->pix_fmt;
	videoFrame->width  = videoCodecCtx->width;
	videoFrame->height = videoCodecCtx->height;

	/* the image can be allocated by any means and av_image_alloc() is
	* just the most convenient way if av_malloc() is to be used */
	int ret = av_image_alloc( videoFrame->data, videoFrame->linesize, videoCodecCtx->width,
							  videoCodecCtx->height, videoCodecCtx->pix_fmt, 32 );
	if ( ret < 0 ) {
		qDebug() << "Could not allocate raw picture buffer.";
		return false;
	}
	
	return true;
}



bool OutputFF::openAudio( QString filename, Profile &prof )
{
	AVCodec *codec;

	AVCodecID codec_id = AV_CODEC_ID_MP2;
	
	filename += ".mp2";
	audioFile = fopen( filename.toLatin1().data(), "wb" );
	if ( !audioFile ) {
		qDebug() << "Could not open" << filename;
		return false;
	}

	// find the video encoder
	codec = avcodec_find_encoder( codec_id );
	if ( !codec ) {
		qDebug() << "Audio encoder not found.";
		return false;
	}

	audioCodecCtx = avcodec_alloc_context3( codec );
	if ( !audioCodecCtx ) {
		qDebug() << "Could not allocate video codec context.";
		return false;
	}

	/* put sample parameters */
	audioCodecCtx->bit_rate = 256000;
	/* check that the encoder supports s16 pcm input */
    audioCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
	/*if ( !check_sample_fmt( codec, audioCodecCtx->sample_fmt ) ) {
		qDebug() << "Audio encoder does not support sample format"
				<< av_get_sample_fmt_name( audioCodecCtx->sample_fmt );
		return false;
	}*/
	/* select other audio parameters supported by the encoder */
    audioCodecCtx->sample_rate = 48000;
    audioCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    audioCodecCtx->channels = av_get_channel_layout_nb_channels( audioCodecCtx->channel_layout );

	/* open it */
	if ( avcodec_open2( audioCodecCtx, codec, NULL ) < 0 ) {
		qDebug() << "Could not open audio codec.";
		return false;
	}

	audioFrame = av_frame_alloc();
	if ( !audioFrame ) {
		qDebug() << "Could not allocate audio frame.";
		return false;
	}
	audioFrame->format = audioCodecCtx->sample_fmt;
	audioFrame->nb_samples  = audioCodecCtx->frame_size;
	audioFrame->channel_layout = audioCodecCtx->channel_layout;

	/* the codec gives us the frame size, in samples,
	* we calculate the size of the samples buffer in bytes */
	audioSamplesSize = av_samples_get_buffer_size( NULL, audioCodecCtx->channels,
												audioCodecCtx->frame_size,
												audioCodecCtx->sample_fmt, 0 );
	if ( audioSamplesSize < 0 ) {
		qDebug() << "Could not get samples buffer size.";
		return false;
	}
	qDebug() << "audioSamplesSize" << audioSamplesSize;
	audioSamples = (uint8_t*)av_malloc( audioSamplesSize );
	if ( !audioSamples ) {
		qDebug() << "Could not allocate samples buffer.";
		return false;
	}
	audioBuffer = (uint8_t*)av_malloc( audioSamplesSize );
	audioBufferLen = 0;
	if ( !audioBuffer ) {
		qDebug() << "Could not allocate audio buffer.";
		return false;
	}
	/* setup the data pointers in the AVFrame */
	int ret = avcodec_fill_audio_frame( audioFrame, audioCodecCtx->channels, 
										audioCodecCtx->sample_fmt,
										(const uint8_t*)audioSamples, audioSamplesSize, 0 );
	if ( ret < 0 ) {
		qDebug() << "Could not setup audio frame";
		return false;
	}
	
	return true;
}



bool OutputFF::init( QString filename, Profile &prof, int vrate, double end )
{
	close();

	if ( !audioFrames || !videoFrames )
		return false;

	if ( !openVideo( filename, prof, vrate ) ) {
		close();
		return false;
	}
	
	if ( !openAudio( filename, prof ) ) {
		close();
		return false;
	}

	endPTS = end;

	return true;
}



void OutputFF::startEncode()
{
	running = true;
	start();
}



bool OutputFF::cancel()
{
	running = false;
	wait();
	return true;
}



void OutputFF::run()
{
	Frame *f;
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	unsigned nVideo = 0, nAudio = 0;
	bool wait;
	bool videoEnd = false;
	QTime time;
	time.start();
	
	while ( running ) {
		wait = true;
		if ( !videoEnd && nVideo == nAudio ) {
			if ( (f = videoFrames->dequeue()) ) {
				encodeVideo( f, nVideo );
				//qDebug() << "N:" << nVideo << f->pts();
				if ( f->pts() > endPTS )
					videoEnd = true;
				if ( time.elapsed() >= 1000 ) {
					emit showFrame( f );
					time.restart();
				}
				else
					f->release();
				++nVideo;
				wait = false;
			}
		}
		if ( nAudio < nVideo ) {
			if ( (f = audioFrames->dequeue()) ) {
				encodeAudio( f, nAudio );
				//qDebug() << "N audio:" << nAudio;
				f->release();
				++nAudio;
				wait = false;
			}
		}
		
		if ( videoEnd && nVideo == nAudio ) {
			qDebug() << "OutputFF::run break";
			break;
		}
		if ( wait ) {
			//qDebug() << "OutputFF::run sleep";
			usleep( 1000 );
		}
	}
	
	AVPacket pkt;
	av_init_packet( &pkt );
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;	
	// get the delayed video frames
	int i = 0, ret;
	for ( int got_output = 1; got_output; i++ ) {
		ret = avcodec_encode_video2( videoCodecCtx, &pkt, NULL, &got_output );
		if ( ret < 0 ) {
			qDebug() << "Error encoding video frame.";
		}

		if ( got_output ) {
			fwrite( pkt.data, 1, pkt.size, videoFile );
			av_free_packet( &pkt );
		}
	}
	// add sequence end code to have a real mpeg file
	fwrite( endcode, 1, sizeof(endcode), videoFile );
	
	av_init_packet( &pkt );
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	i = 0;
	/* get the delayed audio frames */
    for ( int got_output = 1; got_output; i++ ) {
        ret = avcodec_encode_audio2( audioCodecCtx, &pkt, NULL, &got_output );
        if ( ret < 0 ) {
            qDebug() << "Error encoding audio frame.";
        }

        if ( got_output ) {
            fwrite( pkt.data, 1, pkt.size, audioFile );
            av_free_packet( &pkt );
        }
    }

	close();
}



bool OutputFF::encodeVideo( Frame *f, int nFrame )
{
	int ret, got_output;
	AVPacket pkt;
	
	av_init_packet( &pkt );
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	
	uint8_t *buf = f->data();
	int w = f->profile.getVideoWidth();
	int h = f->profile.getVideoHeight();
	int i;
	uint8_t *dst = videoFrame->data[0];
	for ( i = 0; i < h; ++i ) {
		memcpy( dst + i * videoFrame->linesize[0], buf, w );
		buf += w;
	}
	dst = videoFrame->data[1];
	for ( i = 0; i < h / 2; ++i ) {
		memcpy( dst + i * videoFrame->linesize[1], buf, w / 2);
		buf += w / 2;
	}
	dst = videoFrame->data[2];
	for ( i = 0; i < h / 2; ++i ) {
		memcpy( dst + i * videoFrame->linesize[2], buf, w / 2);
		buf += w / 2;
	}

	videoFrame->pts = nFrame;

	// encode the image
	ret = avcodec_encode_video2( videoCodecCtx, &pkt, videoFrame, &got_output );
	if ( ret < 0 ) {
		qDebug() << "Error encoding video frame" << nFrame;
	}

	if ( got_output ) {
		fwrite( pkt.data, 1, pkt.size, videoFile );
		av_free_packet( &pkt );
	}
	
	return true;
}



bool OutputFF::encodeAudio( Frame *f, int nFrame )
{
	int ret, got_output;
	AVPacket pkt;
	
	av_init_packet( &pkt );
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	
	uint8_t *buf = f->data();
	int dataLen = f->audioSamples() * f->profile.getAudioChannels() * 2;
	
	if ( audioBufferLen + dataLen < audioSamplesSize ) {
		memcpy( audioBuffer + audioBufferLen, buf, dataLen );
		audioBufferLen += dataLen;
	}
	else {
		do {
			if ( audioBufferLen > 0 )
				memcpy( audioSamples, audioBuffer, audioBufferLen );
			int size = qMin( dataLen, audioSamplesSize - audioBufferLen );
			memcpy( audioSamples + audioBufferLen, buf, size );
			audioBufferLen = 0;
			dataLen -= size;
			buf += size;
			
			// encode the samples
			ret = avcodec_encode_audio2( audioCodecCtx, &pkt, audioFrame, &got_output );
			if ( ret < 0 ) {
				qDebug() << "Error encoding audio frame" << nFrame;
			}

			if ( got_output ) {
				fwrite( pkt.data, 1, pkt.size, audioFile );
				av_free_packet( &pkt );
			}
		} while ( dataLen >= audioSamplesSize );
		
		if ( dataLen ) {
			memcpy( audioBuffer, buf, dataLen );
			audioBufferLen = dataLen;
		}
	}

	return true;
}
