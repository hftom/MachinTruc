// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include <sys/time.h>

#include <QTimer>

#include "input/ffdecoder.h"

#define DECODEAUDIOSYNC 1
#define DECODEAUDIOPROBE 2



FFDecoder::FFDecoder()
	: formatCtx( NULL ),
	videoCodecCtx( NULL ),
	audioCodecCtx( NULL ),
	swr( NULL ),
	lastChannels( 0 ),
	lastLayout( 0 ),
	videoCodec( NULL ),
	audioCodec( NULL ),
	videoStream( -1 ),
	audioStream( -1 ),
	orientation( 0 ),
	duration( 0 ),
	startTime( 0 ),
	endOfFile( 0 )
{
	FFmpegCommon::getGlobalInstance()->initFFmpeg();

	videoAvframe = av_frame_alloc();
	audioAvframe = av_frame_alloc();
}



FFDecoder::~FFDecoder()
{
	close();

	if ( videoAvframe )
		av_free( videoAvframe );

	if ( audioAvframe )
		av_free( audioAvframe );
}



void FFDecoder::close()
{
	if ( swr )
		swr_free( &swr );

	if ( currentAudioPacket.packet )
		freeCurrentAudioPacket();

	while ( !videoPackets.isEmpty() )
		freePacket( videoPackets.takeFirst() );
	while ( !audioPackets.isEmpty() )
		freePacket( audioPackets.takeFirst() );

	// Close the codec
	if ( videoCodecCtx ) {
		avcodec_close( videoCodecCtx );
		videoCodecCtx = NULL;
	}
	if ( audioCodecCtx ) {
		avcodec_close( audioCodecCtx );
		audioCodecCtx = NULL;
	}

	// Close the video file
	if ( formatCtx ) {
		avformat_close_input( &formatCtx );
		formatCtx = NULL;
	}
}



bool FFDecoder::open( QString fn )
{
	close();

	if ( !ffOpen( fn ) )
		return false;

	duration = formatCtx->duration;
	startTime = (formatCtx->start_time == AV_NOPTS_VALUE) ? 0 : formatCtx->start_time;

	return true;
}


// Critical function. We MUST get the right values.
// Returns true if this file is supported.
bool FFDecoder::probe( QString fn, Profile *prof )
{
	// allow ffOpen to retrieve streams infos
	prof->setHasAudio( true );
	prof->setHasVideo( true );
	// set the audio profile for the following audio/video sync
	setProfile( *prof, *prof );

	if ( !open( fn ) )
		return false;

	prof->setHasAudio( false );
	prof->setHasVideo( false );
	
	if ( haveAudio ) {
		int64_t channel_layout = (audioCodecCtx->channel_layout && audioCodecCtx->channels == av_get_channel_layout_nb_channels(audioCodecCtx->channel_layout)) ?
								audioCodecCtx->channel_layout : av_get_default_channel_layout(audioCodecCtx->channels);
		int bufSize = 1024;
		char buf[ bufSize ];
		buf[0] = 0;
		av_get_channel_layout_string( buf, bufSize, 0, channel_layout );
		prof->setAudioLayoutName( QString( buf ) );
		prof->setAudioCodecName( QString( av_codec_get_codec_descriptor( audioCodecCtx )->name ) );
		prof->setAudioSampleRate( audioCodecCtx->sample_rate );
		prof->setAudioChannels( audioCodecCtx->channels );
		//prof->setAudioFormat( Profile::SAMPLE_FMT_S16 );
		prof->setStreamStartTime( startTime );
		prof->setStreamDuration( duration );
		prof->setHasAudio( true );
		//printf("audio codec name : %s - long name : %s\n", av_codec_get_codec_descriptor( audioCodecCtx )->name, av_codec_get_codec_descriptor( audioCodecCtx )->long_name);
	}
	
	if ( haveVideo ) {
		Frame f( NULL );
		if ( !seekDecodeNext( &f ) )
			return haveAudio;

		// decode some frames
		// check for interlaced
		// and get the average frame rate
		bool ilaced = false;
		bool tff = true;
		double firstpts, lastpts;
		int nframes = 0;
		lastpts = firstpts = f.pts();
		int i;
		for ( i = 0; i < 30; i++ ) {
			if ( !seekDecodeNext( &f ) )
				break;
			++nframes;
			lastpts = f.pts();
			if ( !ilaced ) {
				ilaced = f.profile.getVideoInterlaced();
				tff = f.profile.getVideoTopFieldFirst();
			}
		}

		// too few frames decoded
		if ( nframes < 5 ) {
			return haveAudio;
		}
		// seek back to startTime
		// ffmpeg is not always able to seek the very first frame
		// take as first frame the one that we can seek
		if ( !seek( startTime ) ) {
			return haveAudio;
		}
		// decode this first frame to get the pts
		if ( !seekDecodeNext( &f ) ) {
			return haveAudio;
		}
		// check the pts of the first audio packet
		// if it's late, adjust the first frame
		// ( we MUST be able to sync audio/video at first frame )
		if ( haveAudio ) {
			int loop = 0;
			double pts = f.pts();
			AudioFrame *af = new AudioFrame( Profile::bytesPerChannel( &outProfile ) * outProfile.getAudioChannels(), outProfile.getAudioSampleRate() );
			if ( !decodeAudio( af, DECODEAUDIOPROBE, &pts ) ) {
				return haveAudio;
			}
			delete af;
			while ( pts > f.pts() && loop++ < 250 ) {
				if ( !seekDecodeNext( &f ) ) {
					return haveAudio;
				}
			}
		}

		// FIXME: find the grand universal solution to framerate estimate
		// getting _the_ framerate is very important
		// first try to get something from ffmpeg
		double fps = 0;
		AVStream *st = formatCtx->streams[videoStream];
		AVCodecContext *ctx = videoCodecCtx;
		double tbx = 0.0, tbr = 0.0;
		if ( ctx->time_base.den && ctx->time_base.num )
			tbx = ((double)ctx->time_base.den / (double)ctx->time_base.num);
		if ( st->r_frame_rate.den && st->r_frame_rate.num )
			tbr = (double)st->r_frame_rate.num / (double)st->r_frame_rate.den;
		if ( tbx && tbr ) {
			if ( tbr > 5 && tbr < 121 ) {
				fps = tbr;
				if ( fps > 30 && ilaced )
					fps /= 2.0;
			}
			else if ( tbx > 18 && tbx < 121 )
				fps = tbx / 2.0;
		}
		// compare to usual framerates
		double cfr = 0;
		for ( i = 0; i < NCFR; ++i ) {
			double cfps = CommonFrameRates[i][0] / CommonFrameRates[i][1];
			if ( fabs( cfps - fps ) < fabs( fps - cfr ) ) {
				cfr = cfps;
			}
		}
		if ( fabs( cfr - fps ) < 0.5 ) {
			fps = cfr;
		}
		else if ( nframes > 29 ) {
			// compare to our average frame rate
			double cfps = MICROSECOND * nframes / (lastpts - firstpts);
			if ( fabs( fps - cfps ) < 2 )
				fps = cfps;
		}

		prof->setVideoFrameRate( fps );
		prof->setVideoFrameDuration( MICROSECOND / fps );

		// a probed first frame pts before startTime
		// may cause problem
		double start = startTime;
		if ( startTime == AV_NOPTS_VALUE )
			startTime = start = f.pts();
		else if ( f.pts() > startTime )
			start = f.pts();
		prof->setStreamStartTime( start );

		// get last frame and set stream duration
		prof->setStreamDuration( duration - (prof->getStreamStartTime() - startTime) + prof->getVideoFrameDuration() );
		if ( seek( prof->getStreamStartTime() + prof->getStreamDuration() - MICROSECOND ) ) {
			double p = AV_NOPTS_VALUE;
			while ( seekDecodeNext( &f ) ) {
				if ( f.pts() > p )
					p = f.pts();
			}
			if ( p != AV_NOPTS_VALUE ) {
				prof->setStreamDuration( p - start + prof->getVideoFrameDuration() );
			}
		}
		if ( prof->getStreamDuration() <= 0 ) {
			return haveAudio;
		}

		prof->setVideoCodecName( QString( av_codec_get_codec_descriptor( videoCodecCtx )->name ) );
		prof->setVideoWidth( f.profile.getVideoWidth() );
		prof->setVideoHeight( f.profile.getVideoHeight() );
		prof->setVideoSAR( f.profile.getVideoSAR() );
		prof->setVideoInterlaced( ilaced );
		prof->setVideoTopFieldFirst( tff );
		prof->setVideoColorSpace( f.profile.getVideoColorSpace() );
		prof->setVideoColorPrimaries( f.profile.getVideoColorPrimaries() );
		prof->setVideoChromaLocation( f.profile.getVideoChromaLocation() );
		prof->setVideoColorFullRange( f.profile.getVideoColorFullRange() );
		prof->setHasVideo( true );
	}

	return true;
}



bool FFDecoder::ffOpen( QString fn )
{
	unsigned int i;

	if ( avformat_open_input( &formatCtx, fn.toLocal8Bit().data(), NULL, NULL ) != 0 )
		return false;

	// Retrieve stream information
	if ( avformat_find_stream_info( formatCtx, NULL ) < 0 ) {
		return false;
	}

	videoStream = audioStream = -1;
	haveVideo = haveAudio = false;
	orientation = 0;

	for ( i = 0; i < formatCtx->nb_streams; i++ ) {
		if ( videoStream == -1 && outProfile.hasVideo() && formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO ) {
			if ( (videoCodecCtx = formatCtx->streams[ i ]->codec) ) {
				if ( (videoCodec = avcodec_find_decoder( videoCodecCtx->codec_id )) ) {
					videoStream = i;
					AVStream *st = formatCtx->streams[ i ];
					AVDictionaryEntry *tag = NULL;
					if ( (tag = av_dict_get( st->metadata, "rotate", tag, AV_DICT_IGNORE_SUFFIX )) )
						orientation = QString( tag->value ).toInt();
					haveVideo = true;
				}
			}
		}
		if ( audioStream == -1 && outProfile.hasAudio() && formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO ) {
			if ( (audioCodecCtx = formatCtx->streams[ i ]->codec) ) {
				if ( (audioCodec = avcodec_find_decoder( audioCodecCtx->codec_id )) ) {
					audioStream = i;
					haveAudio = true;
				}
			}
		}
	}

	if ( !haveVideo && !haveAudio )
		return false;

	if ( videoCodecCtx ) {
		if ( avcodec_open2( videoCodecCtx, videoCodec, NULL ) < 0 ) {
			return false; // Could not open codec_id
		}
	}

	if ( audioCodecCtx ) {
		if ( avcodec_open2( audioCodecCtx, audioCodec, NULL ) < 0 ) {
			return false; // Could not open codec_id
		}
		resetAudioResampler();
	}

	endOfFile = 0;

	return true;
}



AVSampleFormat FFDecoder::convertProfileSampleFormat( int f )
{
	if ( f == Profile::SAMPLE_FMT_32F )
		return AV_SAMPLE_FMT_FLT;

	return AV_SAMPLE_FMT_S16;
}



qint64 FFDecoder::convertProfileAudioLayout( int layout )
{
	if ( layout == Profile::LAYOUT_51 )
		return AV_CH_LAYOUT_5POINT1 ;

	return AV_CH_LAYOUT_STEREO;
}



void FFDecoder::resetAudioResampler()
{
	if ( !haveAudio )
		return;

	if ( !audioCodecCtx->channels || !audioCodecCtx->sample_rate || (audioCodecCtx->sample_fmt == AV_SAMPLE_FMT_NONE) ) {
		haveAudio = false;
		return;
	}

	if ( swr )
		swr_free( &swr );

	lastLayout = (audioCodecCtx->channel_layout && audioCodecCtx->channels == av_get_channel_layout_nb_channels(audioCodecCtx->channel_layout)) ?
								audioCodecCtx->channel_layout : av_get_default_channel_layout(audioCodecCtx->channels);
	lastChannels = audioCodecCtx->channels;
	swr = swr_alloc_set_opts( swr, convertProfileAudioLayout( outProfile.getAudioLayout() ), convertProfileSampleFormat( outProfile.getAudioFormat() ),
								outProfile.getAudioSampleRate(), lastLayout, audioCodecCtx->sample_fmt, audioCodecCtx->sample_rate, 0, NULL );
	swr_init( swr );
}



void FFDecoder::flush()
{
	resetAudioResampler();
}



bool FFDecoder::seek( double t )
{
	if ( !formatCtx )
		return false;

	if ( t < startTime )
		t = startTime;

	if ( av_seek_frame( formatCtx, -1, t, AVSEEK_FLAG_BACKWARD ) < 0 ) {
		printf( "av_seek_frame failed.\n" );
		return false;
	}
	else {
		if ( haveVideo )
			avcodec_flush_buffers( videoCodecCtx );
		if ( haveAudio )
			avcodec_flush_buffers( audioCodecCtx );

		if ( currentAudioPacket.packet )
			freeCurrentAudioPacket();

		while ( !videoPackets.isEmpty() )
			freePacket( videoPackets.takeFirst() );
		while ( !audioPackets.isEmpty() )
			freePacket( audioPackets.takeFirst() );
	}

	endOfFile = 0;
	return true;
}



bool FFDecoder::seekDecodeNext( Frame *f )
{
	bool ret = decodeVideo( f );
	return ret;
}



bool FFDecoder::seekTo( double p, Frame *f, AudioFrame *af )
{
	if ( !formatCtx || ( p < startTime ) )
		return false;

	flush();

	double timestamp = p;
	double lastpts = p;
	bool before;
	int loop = 0, maxloop = 10;
	int seekinc = MICROSECOND / inProfile.getVideoFrameDuration();
	int timestampinc = 2, audiotimestampinc = 2;
	double tail = inProfile.getStreamStartTime() + inProfile.getStreamDuration() - MICROSECOND;

	if ( !haveVideo ) {
		if ( haveAudio ) {
			while ( loop++ < maxloop ) {
				seek( timestamp );
				if ( !decodeAudio( af, DECODEAUDIOSYNC, &p ) ) {
					printf("! decodeAudio\n");
					timestamp -= MICROSECOND;
				}
				else
					break;
			}
		}
	}
	else {
		while ( loop < maxloop ) {
			before = false;
			seek( timestamp );
			if ( !seekDecodeNext( f ) ) {
				if ( p > tail && timestamp > startTime ) {
					timestamp -= MICROSECOND / 2.0;
					++loop;
					continue;
				}
				printf("seekTo !decodeVideo\n");
				return false;
			}
			printf("decoded PTS=%f, timestamp=%f, wanted PTS=%f\n", f->pts(), timestamp, p );

			double cur = f->pts();
			if ( cur < p )
				before = true; // do not seek back further if we are already before p
			double delta = fabs(cur - p);
			double hdur = (f->profile.getVideoFrameDuration() / 2.);
			if ( cur == lastpts )
				++loop;

			// If p is near the end, we can get funky negative pts with some formats;
			// if so, seek back.
			if ( p >= 0 && cur < 0 ) {
				timestamp -= seekinc * hdur;
			}
			else if ( cur > p ) {
				if ( delta > hdur && !before )
					timestamp -= (timestampinc++ * (cur - p));
				else if ( haveAudio && !decodeAudio( af, DECODEAUDIOSYNC, &cur ) )
					timestamp -= seekinc * hdur;
				else
					break;
			}
			else if ( delta > hdur ) {
				do {
					if ( !seekDecodeNext( f ) ) {
						return false;
					}
					cur = f->pts();
					if ( cur < p )
						before = true; // do not seek back further if we are already before p
					delta = fabs(cur - p);
					hdur = (f->profile.getVideoFrameDuration() / 2.);
					if ( cur == lastpts )
						++loop;
					printf("frame PTS=%f, wanted PTS=%f, hdur=%f, delta=%f\n", cur, p, hdur, delta );
				} while ( (cur < p) && (delta > hdur) && (loop < maxloop) );

				if ( (cur > p) && ( delta > hdur && !before ) )
					timestamp -= (audiotimestampinc++ * (cur - p));
				else if ( haveAudio && !decodeAudio( af, DECODEAUDIOSYNC, &cur ) )
					timestamp -= seekinc * hdur;
				else
					break;
			}
			else if ( haveAudio && !decodeAudio( af, DECODEAUDIOSYNC, &cur ) )
				timestamp -= seekinc * hdur;
			else
				break;

			lastpts = cur;
		}
		p = f->pts();
		//resample( f );
	}

	return true;
}



bool FFDecoder::decodeVideo( Frame *f )
{
	int gotFrame = 0;
	double vpts = 0;

	while ( 1 ) {
		while ( videoPackets.isEmpty() && !(endOfFile & EofVideoPacket) ) {
			if ( !getPacket() ) {
				endOfFile |= EofVideoPacket;
				break;
			}
		}
		AVPacket *packet;
		if ( endOfFile & EofVideoPacket ) {
			// get the last few frames
			packet = (AVPacket*)malloc( sizeof(AVPacket) );
			av_init_packet( packet );
			packet->data = NULL;
			packet->size = 0;
		}
		else {
			if ( !(packet = videoPackets.dequeue()) )
				return false;
		}
		int len = avcodec_decode_video2( videoCodecCtx, videoAvframe, &gotFrame, packet );
		if ( len >= 0 && gotFrame ) {
			AVStream *st = formatCtx->streams[videoStream];
			double tb = av_q2d( st->time_base ) * AV_TIME_BASE;
			vpts = av_frame_get_best_effort_timestamp( videoAvframe ) * tb;

			double ratio = 1.0;
			if ( st->sample_aspect_ratio.num && av_cmp_q(st->sample_aspect_ratio, st->codec->sample_aspect_ratio) ) {
				ratio = (double)st->sample_aspect_ratio.num / (double)st->sample_aspect_ratio.den;
			}
			else if ( videoAvframe->sample_aspect_ratio.num && videoAvframe->sample_aspect_ratio.den ) {
				ratio = (double)videoAvframe->sample_aspect_ratio.num / (double)videoAvframe->sample_aspect_ratio.den;
			}

			double dur = inProfile.getVideoFrameDuration();
			if ( videoAvframe->repeat_pict ) {
				//printf("repeat_pict = %d\n", videoAvframe->repeat_pict);
				dur += dur * videoAvframe->repeat_pict / 2.0;
			}

			bool ok = makeFrame( f, ratio, vpts, dur );
			
			freePacket( packet );
			return ok;
		}

		freePacket( packet );
		if ( endOfFile & EofVideoPacket ) {
			endOfFile |= EofVideo;
			return false;
		}
	}
	return false;
}



bool FFDecoder::makeFrame( Frame *f, double ratio, double pts, double dur )
{
	f->profile.setVideoColorFullRange( videoCodecCtx->color_range == AVCOL_RANGE_JPEG );

	int height = videoCodecCtx->height;
	if ( height == 1088 )
		height = 1080;

	switch ( videoCodecCtx->colorspace ) {
		case AVCOL_SPC_SMPTE240M:
		case AVCOL_SPC_BT709:
			f->profile.setVideoColorSpace( Profile::SPC_709 );
			break;
		case AVCOL_SPC_BT470BG:
			f->profile.setVideoColorSpace( Profile::SPC_601_625 );
			break;
		case AVCOL_SPC_SMPTE170M:
			f->profile.setVideoColorSpace( Profile::SPC_601_525 );
			break;
		default:
			f->profile.setVideoColorSpace( (videoCodecCtx->width * height > 1280 * 576) ? Profile::SPC_709 : Profile::SPC_601_625 );
	}

	switch ( videoCodecCtx->color_primaries ) {
		case AVCOL_PRI_BT470BG:
			f->profile.setVideoColorPrimaries( Profile::PRI_601_625 );
			break;
		case AVCOL_PRI_SMPTE170M:
		case AVCOL_PRI_SMPTE240M:
			f->profile.setVideoColorPrimaries( Profile::PRI_601_525 );
			break;
		case AVCOL_PRI_BT709:
		default:
			f->profile.setVideoColorPrimaries( Profile::PRI_709 );
	}

	switch ( videoCodecCtx->color_trc ) {
		case AVCOL_TRC_SMPTE170M:
			f->profile.setVideoGammaCurve( Profile::GAMMA_601 );
			break;
		case AVCOL_TRC_BT709:
		default:
			f->profile.setVideoGammaCurve( Profile::GAMMA_709 );
	}


	switch ( videoCodecCtx->chroma_sample_location ) {
		case AVCHROMA_LOC_CENTER:
			f->profile.setVideoChromaLocation( Profile::LOC_CENTER );
			break;
		case AVCHROMA_LOC_TOPLEFT:
			f->profile.setVideoChromaLocation( Profile::LOC_TOPLEFT );
			break;
		case AVCHROMA_LOC_UNSPECIFIED:
		case AVCHROMA_LOC_LEFT:
		default:
			f->profile.setVideoChromaLocation( Profile::LOC_LEFT );
	}

	switch ( videoAvframe->format ) {
		case AV_PIX_FMT_YUVJ420P:
		case AV_PIX_FMT_YUV420P: {
			f->setVideoFrame( Frame::YUV420P, videoCodecCtx->width, height,
				ratio, videoAvframe->interlaced_frame, videoAvframe->top_field_first, pts, dur, orientation );
			int i;
			uint8_t *buf = f->data();
			for ( i = 0; i < height; i++ ) {
				memcpy( buf, videoAvframe->data[0] + (videoAvframe->linesize[0] * i), videoCodecCtx->width );
				buf += videoCodecCtx->width;
			}
			for ( i = 0; i < height / 2; i++ ) {
				memcpy( buf, videoAvframe->data[1] + (videoAvframe->linesize[1] * i), videoCodecCtx->width / 2 );
				buf += videoCodecCtx->width / 2;
			}
			for ( i = 0; i < height / 2; i++ ) {
				memcpy( buf, videoAvframe->data[2] + (videoAvframe->linesize[2] * i), videoCodecCtx->width / 2 );
				buf += videoCodecCtx->width / 2;
			}
			break;
		}
		case AV_PIX_FMT_YUVJ422P:
		case AV_PIX_FMT_YUV422P: {
			f->setVideoFrame( Frame::YUV422P, videoCodecCtx->width, height,
				ratio, videoAvframe->interlaced_frame, videoAvframe->top_field_first, pts, dur, orientation );
			int i;
			uint8_t *buf = f->data();
			for ( i = 0; i < height; i++ ) {
				memcpy( buf, videoAvframe->data[0] + (videoAvframe->linesize[0] * i), videoCodecCtx->width );
				buf += videoCodecCtx->width;
			}
			for ( i = 0; i < height; i++ ) {
				memcpy( buf, videoAvframe->data[1] + (videoAvframe->linesize[1] * i), videoCodecCtx->width / 2 );
				buf += videoCodecCtx->width / 2;
			}
			for ( i = 0; i < height; i++ ) {
				memcpy( buf, videoAvframe->data[2] + (videoAvframe->linesize[2] * i), videoCodecCtx->width / 2 );
				buf += videoCodecCtx->width / 2;
			}
			break;
		}
		default: {
			printf("AV_PIX_FMT not supported\n");
			return false;
		}
	}

	return true;
}



bool FFDecoder::decodeAudio( AudioFrame *f, int sync, double *pts )
{
	int gotFrame = 0;
	int writtenSamples = 0;

	while ( 1 ) {
		if ( !currentAudioPacket.packet ) {
			while ( audioPackets.isEmpty() ) {
				if ( !getPacket() ) {
					// get the last few samples
					int n = swr_get_delay( swr, outProfile.getAudioSampleRate() ) * outProfile.getAudioChannels();
					uint8_t* dst = f->write( writtenSamples, n );
					writtenSamples += swr_convert( swr, &dst, n, 0, 0 );
					if ( writtenSamples )
						f->writeDone( 0, writtenSamples );
					endOfFile |= EofAudio;
					return false;
				}
			}
			currentAudioPacket.set( audioPackets.dequeue() );
		}
		int len=0;
		while ( len >= 0 && currentAudioPacket.packet->size > 0 ) {
			len = avcodec_decode_audio4( audioCodecCtx, audioAvframe, &gotFrame, currentAudioPacket.packet );
			if ( len >= 0 && gotFrame ) {
				double vpts;
				if ( currentAudioPacket.pts != AV_NOPTS_VALUE )
					vpts = currentAudioPacket.pts;
				else {
					vpts = av_q2d( formatCtx->streams[audioStream]->time_base ) * AV_TIME_BASE ;
					vpts *= av_frame_get_best_effort_timestamp( audioAvframe );
				}

				if ( !sync ) {
					// get the pts of the resampled chunk
					vpts -= swr_get_delay( swr, outProfile.getAudioSampleRate() ) * MICROSECOND / outProfile.getAudioSampleRate();
				}

				// roundup estimate
				int outSamples = av_rescale_rnd( swr_get_delay( swr, audioCodecCtx->sample_rate ) + audioAvframe->nb_samples, outProfile.getAudioSampleRate(), audioCodecCtx->sample_rate, AV_ROUND_UP );
				// FIXME:nasty hack to avoid ringbuffer overflow
				outSamples *= outProfile.getAudioChannels();

				// audio channels and layout may have changed
				// if so, we flush swr internal buffer and reset
				int64_t layout = (audioCodecCtx->channel_layout && audioCodecCtx->channels == av_get_channel_layout_nb_channels(audioCodecCtx->channel_layout)) ?
								audioCodecCtx->channel_layout : av_get_default_channel_layout(audioCodecCtx->channels);
				if ( audioCodecCtx->channels != lastChannels || layout != lastLayout ) {
					//printf("channels=%d, lastChannels=%d\n", audioCodecCtx->channels, lastChannels);
					int n = swr_get_delay( swr, outProfile.getAudioSampleRate() ) * outProfile.getAudioChannels();
					uint8_t* dst = f->write( writtenSamples, n );
					writtenSamples += swr_convert( swr, &dst, n, 0, 0 );
					resetAudioResampler();
				}

				// We have to convert in order to get the exact number of out samples
				// and swr_convert writes directly into the ringbuffer.
				// So, if all samples have to be skipped, don't call f->writeDone,
				// thus the write pointer isn't increased and next time we will overwrite this chunk.
				uint8_t* dst = f->write( writtenSamples, outSamples );
				outSamples = swr_convert( swr, &dst, outSamples, (const uint8_t**)&audioAvframe->data[0], audioAvframe->nb_samples );
				writtenSamples += outSamples;
				if ( sync ) {
					if ( sync == DECODEAUDIOPROBE ) {
						*pts = vpts;
						return true;
					}
					// how much samples to skip?
					int ns = (*pts - vpts) * (double)outProfile.getAudioSampleRate() / MICROSECOND;
					//printf("pts=%f, vpts=%f, ns=%d, sr=%d, outSamples=%d\n", *pts, vpts, ns, outProfile.getAudioSampleRate(), outSamples);
					if ( ns < 0 ) {
						// we have to seek back again
						freeCurrentAudioPacket();
						return false;
					}
					else if ( ns < outSamples ) {
						// Tell the ringbuffer that "ns" samples are skipped
						f->writeDone( *pts, writtenSamples, ns );
						if ( (currentAudioPacket.packet->size - len) <= 0 )
							freeCurrentAudioPacket();
						else {
							shiftCurrentAudioPacket( len );
							shiftCurrentAudioPacketPts( vpts );
						}
						return true;
					}
					else {
						writtenSamples = 0;
						shiftCurrentAudioPacketPts( vpts );
					}
				}
				else {
					f->writeDone( vpts, writtenSamples );
					if ( (currentAudioPacket.packet->size - len) <= 0 )
						freeCurrentAudioPacket();
					else {
						shiftCurrentAudioPacket( len );
						shiftCurrentAudioPacketPts( vpts );
					}
					return true;
				}
			}
			shiftCurrentAudioPacket( len );
		}
		freeCurrentAudioPacket();
	}

	return false;
}



void FFDecoder::shiftCurrentAudioPacketPts( double pts )
{
	currentAudioPacket.pts = pts + (MICROSECOND * audioAvframe->nb_samples / audioCodecCtx->sample_rate);
}



void FFDecoder::shiftCurrentAudioPacket( int len )
{
	currentAudioPacket.packet->size -= len;
	currentAudioPacket.packet->data += len;
}



void FFDecoder::freeCurrentAudioPacket()
{
	currentAudioPacket.reset();
	freePacket( currentAudioPacket.packet );
	currentAudioPacket.free();
}



bool FFDecoder::getPacket()
{
	if ( endOfFile )
		return false;
	AVPacket *packet = (AVPacket*)malloc( sizeof(AVPacket) );
	if ( !packet ) {
		return false;
	}
	av_init_packet( packet );
	if ( av_read_frame( formatCtx, packet ) < 0 ) {
		freePacket( packet );
		endOfFile |= EofPacket;
		return false;
	}
	if ( packet->stream_index == videoStream ) {
		if ( !av_dup_packet( packet ) )
			videoPackets.enqueue( packet );
		else
			freePacket( packet );
	}
	else if ( packet->stream_index == audioStream ) {
		if ( !av_dup_packet( packet ) )
			audioPackets.enqueue( packet );
		else
			freePacket( packet );
	}
	else {
		freePacket( packet );
	}
	return true;
}



void FFDecoder::freePacket( AVPacket *packet )
{
	av_free_packet( packet );
	free( packet );
}