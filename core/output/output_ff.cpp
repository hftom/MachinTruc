#include <QDebug>
#include <QTime>

#include "output_ff.h"



OutputFF::OutputFF( MQueue<Frame*> *vf, MQueue<Frame*> *af )
	: audioFrames( af ),
	videoFrames( vf ),
	running( false ),
	formatCtx( NULL ),
	videoStream( NULL ),
	videoFrame( NULL ),
	audioStream( NULL ),
	audioFrame( NULL ),
	audioSamples( NULL ),
	audioSamplesSize( 0 ),
	audioBuffer( NULL ),
	audioBufferLen( 0 ),
	firstAudioPts( 0 ),
	endPTS( 0 ),
	showFrameProgress( true )
{
	FFmpegCommon::getGlobalInstance()->initFFmpeg();
}



OutputFF::~OutputFF()
{
	close();
}



void OutputFF::close()
{	
	if ( videoStream ) {
		avcodec_close( videoStream->codec );
		videoStream = NULL;
	}
	if ( videoFrame ) {
		av_frame_free( &videoFrame );
		videoFrame = NULL;
	}

	if ( audioStream ) {
		avcodec_close( audioStream->codec );
		audioStream = NULL;
	}
	if ( audioSamples ) {
		av_freep( &audioSamples );
	}
	if ( audioBuffer ) {
		av_freep( &audioBuffer );
	}
	if ( audioFrame ) {
		av_frame_free( &audioFrame );
		audioFrame = NULL;
	}
	
	if ( formatCtx ) {
		avformat_free_context( formatCtx );
		formatCtx = NULL;
	}
}



bool OutputFF::openVideo( Profile &prof, int vrate, bool mpeg )
{
	AVCodecID vcodec_id = AV_CODEC_ID_H264;
	if ( mpeg )
		vcodec_id = AV_CODEC_ID_MPEG2VIDEO;
	AVCodec *codec = avcodec_find_encoder( vcodec_id );
	if ( !codec ) {
		qDebug() << "Could not find video encoder.";
		return false;
	}
	videoStream = avformat_new_stream( formatCtx, codec );
	if ( !videoStream ) {
		qDebug() << "Could not allocate video stream.";
		return false;
	}
	videoStream->id = formatCtx->nb_streams - 1;
	
	AVCodecContext *videoCodecCtx = videoStream->codec;
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
	videoCodecCtx->gop_size = prof.getVideoFrameRate();
	videoCodecCtx->max_b_frames = 2;
	videoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	int sw = (double)prof.getVideoWidth() * prof.getVideoSAR();
	int sh = prof.getVideoWidth();
	videoCodecCtx->sample_aspect_ratio = (AVRational){ sw, sh };
	
	videoStream->sample_aspect_ratio = videoCodecCtx->sample_aspect_ratio;
	videoStream->time_base = videoCodecCtx->time_base;

	//if ( codec_id == AV_CODEC_ID_H264 )
		//av_opt_set( videoCodecCtx->priv_data, "preset", "veryslow", 0 );
	
	if ( formatCtx->oformat->flags & AVFMT_GLOBALHEADER )
		videoCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;

	// open it
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

	// allocate the buffers for the frame data
	int ret = av_frame_get_buffer( videoFrame, 32 );
	if ( ret < 0 ) {
		qDebug() << "Could not allocate video frame buffers.";
		return false;
	}
	
	return true;
}



bool OutputFF::openAudio( Profile &prof )
{
	AVCodecID codec_id = AV_CODEC_ID_MP2;

	// find the video encoder
	AVCodec *codec = avcodec_find_encoder( codec_id );
	if ( !codec ) {
		qDebug() << "Audio encoder not found.";
		return false;
	}
	
	audioStream = avformat_new_stream( formatCtx, codec );
	if ( !audioStream ) {
		qDebug() << "Could not allocate audio stream.";
		return false;
	}
	audioStream->id = formatCtx->nb_streams - 1;

	AVCodecContext *audioCodecCtx = audioStream->codec;
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
	
	audioStream->time_base = (AVRational){ 1, audioCodecCtx->sample_rate };
	
	if ( formatCtx->oformat->flags & AVFMT_GLOBALHEADER )
		audioCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;

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
	audioFrame->sample_rate = audioCodecCtx->sample_rate;

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



bool OutputFF::openFormat( QString filename, Profile &prof, int vrate, bool mpeg )
{
	int ret;

	avformat_alloc_output_context2( &formatCtx, NULL, "matroska", filename.toLatin1().data()  );
	
	if ( !formatCtx ) {
		qDebug() << "Could not open format context.";
		return false;
	}
	
	if ( !openVideo( prof, vrate, mpeg ) ) {
		close();
		return false;
	}

	if ( !openAudio( prof ) ) {
		close();
		return false;
	}
	
	// open the output file, if needed
	if ( !( formatCtx->oformat->flags & AVFMT_NOFILE ) ) {
		ret = avio_open( &formatCtx->pb, filename.toLatin1().data(), AVIO_FLAG_WRITE );
		if ( ret < 0 ) {
			qDebug() << "Could not open" << filename;
			close();
			return false;
		}
	}
    
	// Write the stream header, if any.
	ret = avformat_write_header( formatCtx, NULL );
	if (ret < 0) {
		qDebug() << "Error occurred when opening output file.";
		close();
		return false;
	}
	
	return true;
}



bool OutputFF::init( QString filename, Profile &prof, int vrate, bool mpeg, double end )
{
	close();

	if ( !audioFrames || !videoFrames )
		return false;

	if ( !openFormat( filename, prof, vrate, mpeg ) )
		return false;	

	endPTS = end;

	return true;
}



void OutputFF::startEncode( bool show )
{
	showFrameProgress = show;
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
				if ( showFrameProgress && time.elapsed() >= 1000 ) {
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
				if ( !nAudio )
					firstAudioPts = f->pts();
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
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	av_init_packet( &pkt );
	// get the delayed video frames
	int i = 0, ret;
	for ( int got_output = 1; got_output; i++ ) {
		ret = avcodec_encode_video2( videoStream->codec, &pkt, NULL, &got_output );
		if ( ret < 0 ) {
			qDebug() << "Error encoding video frame.";
		}
		
		if ( got_output ) {
			// rescale output packet timestamp values from codec to stream timebase
			av_packet_rescale_ts( &pkt, videoStream->codec->time_base, videoStream->time_base );
			pkt.stream_index = videoStream->index;
			// Write the compressed frame to the media file.
			ret = av_interleaved_write_frame( formatCtx, &pkt );
			if ( ret < 0 )
				qDebug() << "Error while writing video frame.";
		}
	}
	
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	av_init_packet( &pkt );
	i = 0;
	/* get the delayed audio frames */
    for ( int got_output = 1; got_output; i++ ) {
        ret = avcodec_encode_audio2( audioStream->codec, &pkt, NULL, &got_output );
        if ( ret < 0 ) {
            qDebug() << "Error encoding audio frame.";
        }

        if ( got_output ) {
			// rescale output packet timestamp values from codec to stream timebase
			av_packet_rescale_ts( &pkt, audioStream->codec->time_base, audioStream->time_base );
			pkt.stream_index = audioStream->index;
			// Write the compressed frame to the media file.
			ret = av_interleaved_write_frame( formatCtx, &pkt );
			if ( ret < 0 )
				qDebug() << "Error while writing audio frame.";
		}
    }
    
	/* Write the trailer, if any. The trailer must be written before you
	* close the CodecContexts open when you wrote the header; otherwise
	* av_write_trailer() may try to use memory that was freed on
	* av_codec_close(). */
	av_write_trailer( formatCtx );

	close();
}



bool OutputFF::encodeVideo( Frame *f, int nFrame )
{
	int ret, got_output;
	AVPacket pkt;
	
	pkt.data = NULL;    // packet data will be allocated by the encoder
	pkt.size = 0;
	av_init_packet( &pkt );
	
	/* when we pass a frame to the encoder, it may keep a reference to it
	* internally;
	* make sure we do not overwrite it here */
	av_frame_make_writable( videoFrame );
	
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
	ret = avcodec_encode_video2( videoStream->codec, &pkt, videoFrame, &got_output );
	if ( ret < 0 ) {
		qDebug() << "Error encoding video frame" << nFrame;
	}

	if ( got_output ) {
		// rescale output packet timestamp values from codec to stream timebase
		av_packet_rescale_ts( &pkt, videoStream->codec->time_base, videoStream->time_base );
		pkt.stream_index = videoStream->index;
		// Write the compressed frame to the media file.
		ret = av_interleaved_write_frame( formatCtx, &pkt );
		if ( ret < 0 )
			qDebug() << "Error while writing video frame.";
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
	
	/* when we pass a frame to the encoder, it may keep a reference to it
	* internally;
	* make sure we do not overwrite it here */
	av_frame_make_writable( audioFrame );
		
	uint8_t *buf = f->data();
	int dataLen = f->audioSamples() * f->profile.getAudioChannels() * 2;
	
	if ( audioBufferLen + dataLen < audioSamplesSize ) {
		memcpy( audioBuffer + audioBufferLen, buf, dataLen );
		audioBufferLen += dataLen;
	}
	else {
		int totalSamples = ( f->pts() - firstAudioPts ) * 48000 / MICROSECOND;
		do {
			if ( audioBufferLen > 0 ) {
				totalSamples -= audioBufferLen / 4;
				memcpy( audioSamples, audioBuffer, audioBufferLen );
			}
			int size = qMin( dataLen, audioSamplesSize - audioBufferLen );
			memcpy( audioSamples + audioBufferLen, buf, size );
			audioBufferLen = 0;
			dataLen -= size;
			buf += size; 
			
			audioFrame->pts = av_rescale_q( totalSamples,
											(AVRational){1, audioStream->codec->sample_rate},
											audioStream->codec->time_base );
			
			// encode the samples
			ret = avcodec_encode_audio2( audioStream->codec, &pkt, audioFrame, &got_output );
			if ( ret < 0 ) {
				qDebug() << "Error encoding audio frame" << nFrame;
			}
			
			if ( got_output ) {
				// rescale output packet timestamp values from codec to stream timebase
				av_packet_rescale_ts( &pkt, audioStream->codec->time_base, audioStream->time_base );
				pkt.stream_index = audioStream->index;
				// Write the compressed frame to the media file.
				ret = av_interleaved_write_frame( formatCtx, &pkt );
				if ( ret < 0 )
					qDebug() << "Error while writing audio frame.";
			}
			
			totalSamples += audioSamplesSize / 4;

		} while ( dataLen >= audioSamplesSize );
		
		if ( dataLen ) {
			memcpy( audioBuffer, buf, dataLen );
			audioBufferLen = dataLen;
		}
	}

	return true;
}
