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
		av_frame_free( &videoAvframe );

	if ( audioAvframe )
		av_frame_free( &audioAvframe );
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
		prof->setVideoGammaCurve( f.profile.getVideoGammaCurve() );
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
					videoCodecCtx->thread_count = 0;
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
		AVDictionary *opts = NULL;
		av_dict_set( &opts, "refcounted_frames", "1", 0 );
		if ( avcodec_open2( videoCodecCtx, videoCodec, &opts ) < 0 ) {
			return false; // Could not open codec_id
		}
		yadif.reset( outProfile.getVideoFrameRate() > inProfile.getVideoFrameRate(), videoStream, formatCtx, videoCodecCtx );
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
	swr = swr_alloc_set_opts( swr, FFmpegCommon::getGlobalInstance()->convertProfileAudioLayout( outProfile.getAudioLayout() ),
							  FFmpegCommon::getGlobalInstance()->convertProfileSampleFormat( outProfile.getAudioFormat() ),
							  outProfile.getAudioSampleRate(), lastLayout, audioCodecCtx->sample_fmt, audioCodecCtx->sample_rate, 0, NULL );
	swr_init( swr );
}



void FFDecoder::flush()
{
	resetAudioResampler();
	if ( haveVideo && doYadif )
		yadif.reset( doYadif > Yadif1X, videoStream, formatCtx, videoCodecCtx );
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
			if ( doYadif )
				yadif.reset( doYadif > Yadif1X, videoStream, formatCtx, videoCodecCtx );
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
			//printf("decoded PTS=%f, timestamp=%f, wanted PTS=%f\n", f->pts(), timestamp, p );

			double cur = f->pts();
			if ( cur < p )
				before = true; // do not seek back further if we are already before p
			double delta = fabs(cur - p);
			double hdur = (f->profile.getVideoFrameDuration() / 2.);
			if ( cur == lastpts )
				++loop;

			/*// If p is near the end, we can get funky negative pts with some formats;
			// if so, seek back.
			if ( p >= 0 && cur < 0 ) {
				timestamp -= seekinc * hdur;
			}
			else */if ( cur > p ) {
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
					//printf("frame PTS=%f, wanted PTS=%f, hdur=%f, delta=%f\n", cur, p, hdur, delta );
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
	}

	return true;
}



bool FFDecoder::decodeVideo( Frame *f )
{
	int gotFrame = 0;
	double vpts = 0;

	while ( 1 ) {
		if ( doYadif ) {
			double pts, dur, ratio;
			AVFrame *ya = yadif.pullFrame( pts, dur, ratio );
			if ( ya ) {
				bool ok = makeFrame( f, ya, ratio, pts, dur );
				av_frame_unref( ya );
				return ok;
			}
			else if ( yadif.eof ) {
				endOfFile |= EofVideo;
				return false;
			}
		}
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

			if ( doYadif ) {
				yadif.pushFrame( videoAvframe, vpts, dur, ratio );
				av_frame_unref( videoAvframe );
				freePacket( packet );
			}
			else {
				bool ok = makeFrame( f, videoAvframe, ratio, vpts, dur );
				av_frame_unref( videoAvframe );
				freePacket( packet );
				return ok;
			}
		}
		else {
			freePacket( packet );
			if ( endOfFile & EofVideoPacket ) {
				if ( doYadif ) {
					yadif.pushFrame( NULL, 0, 0, 0 );
				}
				else {
					endOfFile |= EofVideo;
					return false;
				}
			}
		}
	}
	return false;
}



bool FFDecoder::makeFrame( Frame *f, AVFrame *avFrame, double ratio, double pts, double dur )
{
	f->profile.setVideoColorFullRange( videoCodecCtx->color_range == AVCOL_RANGE_JPEG );

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
			f->profile.setVideoColorSpace( (videoCodecCtx->width * videoCodecCtx->height > 1280 * 576) ? Profile::SPC_709 : Profile::SPC_601_625 );
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

	Frame::DataType formatType;
	int ph[3] = {
		videoCodecCtx->height,
		videoCodecCtx->height,
		videoCodecCtx->height
	};
	int pw[3] = {
		videoCodecCtx->width,
		videoCodecCtx->width / 2,
		videoCodecCtx->width / 2
	};
	int bitDepth = 8;
	switch ( avFrame->format ) {
		case AV_PIX_FMT_YUVJ420P:
		case AV_PIX_FMT_YUV420P: {
			formatType = Frame::YUV420P;
			ph[1] /= 2;
			ph[2] /= 2;
			break;
		}
		case AV_PIX_FMT_YUVJ422P:
		case AV_PIX_FMT_YUV422P: {
			formatType = Frame::YUV422P;
			break;
		}
		case AV_PIX_FMT_YUVJ444P:
		case AV_PIX_FMT_YUV444P: {
			formatType = Frame::YUV444P;
			pw[1] *= 2;
			pw[2] *= 2;
			break;
		}
		case AV_PIX_FMT_YUV420P10LE: {
			formatType = Frame::YUV420P;
			ph[1] /= 2;
			ph[2] /= 2;
			bitDepth = 10;
			break;
		}
		case AV_PIX_FMT_YUV422P10LE: {
			formatType = Frame::YUV422P;
			bitDepth = 10;
			break;
		}
		case AV_PIX_FMT_YUV444P10LE: {
			formatType = Frame::YUV444P;
			pw[1] *= 2;
			pw[2] *= 2;
			bitDepth = 10;
			break;
		}
		case AV_PIX_FMT_YUV420P12LE: {
			formatType = Frame::YUV420P;
			ph[1] /= 2;
			ph[2] /= 2;
			bitDepth = 12;
			break;
		}
		case AV_PIX_FMT_YUV422P12LE: {
			formatType = Frame::YUV422P;
			bitDepth = 12;
			break;
		}
		case AV_PIX_FMT_YUV444P12LE: {
			formatType = Frame::YUV444P;
			pw[1] *= 2;
			pw[2] *= 2;
			bitDepth = 12;
			break;
		}
		case AV_PIX_FMT_RGB32: {
			formatType = Frame::RGBA;
			pw[0] *= 4;
			break;
		}
		default: {
			printf("AV_PIX_FMT not supported\n");
			return false;
		}
	}

	f->setVideoFrame( formatType, videoCodecCtx->width, videoCodecCtx->height,
					  ratio, avFrame->interlaced_frame, avFrame->top_field_first, pts, dur, orientation, 0, bitDepth);

	if (formatType >= Frame::RGBA) {
		uint8_t *buf = f->data();
		for ( int i = 0; i < videoCodecCtx->height; ++i) {
			memcpy( buf, avFrame->data[0] + (avFrame->linesize[0] * i), pw[0]);
			buf += pw[0];
		}
	}
	else {
		copyYUVPlanar(f->data(), avFrame, ph, pw, bitDepth);
	}

	return true;
}


// removes ffmpeg padding
void FFDecoder::copyYUVPlanar(uint8_t *buf, AVFrame *avFrame, int ph[3], int pw[3], int bits)
{
	int i;
	int bytes = bits > 8 ? 2 : 1;
	pw[0] *= bytes;
	pw[1] *= bytes;
	pw[2] *= bytes;

	for ( i = 0; i < ph[0]; i++ ) {
		memcpy( buf, avFrame->data[0] + (avFrame->linesize[0] * i), pw[0] );
		buf += pw[0];
	}
	for ( i = 0; i < ph[1]; i++ ) {
		memcpy( buf, avFrame->data[1] + (avFrame->linesize[1] * i), pw[1] );
		buf += pw[1];
	}
	for ( i = 0; i < ph[2]; i++ ) {
		memcpy( buf, avFrame->data[2] + (avFrame->linesize[2] * i), pw[2] );
		buf += pw[2];
	}
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
				//outSamples *= outProfile.getAudioChannels();

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
				// but we want swr_convert to write directly into the AudioFrame buffer.
				// So, if all samples have to be skipped, don't call f->writeDone,
				// thus the write pointer isn't increased and next time we will overwrite this chunk.
				uint8_t* dst = f->write( writtenSamples, outSamples );
				outSamples = swr_convert( swr, &dst, outSamples, (const uint8_t**)audioAvframe->extended_data, audioAvframe->nb_samples );
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
						// Tell AudioFrame that "ns" samples are skipped
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



// yadif filter
Yadif::Yadif() : eof(false), twice(false), bufferSinkCtx(NULL), bufferSrcCtx(NULL), filterGraph(NULL)
{
	filterFrame = av_frame_alloc();
}



Yadif::~Yadif()
{
	if ( filterGraph ) {
		avfilter_graph_free( &filterGraph );
		filterGraph = NULL;
	}
	av_frame_free( &filterFrame );
}



bool Yadif::pushFrame( AVFrame *f, double pts, double duration, double ratio )
{
	if ( !f )
		eof = true;
	else
		ar = ratio;
	int ret = av_buffersrc_add_frame_flags( bufferSrcCtx, f, AV_BUFFERSRC_FLAG_KEEP_REF );
	if ( ret < 0 ) {
		char es[128];
		av_strerror( ret, es, 128 );
		qDebug() << "Error while feeding the filtergraph:" << es;
		return false;
	}
	if ( twice ) {
		ptsQueue.enqueue( pts );
		durationQueue.enqueue( duration / 2.0 );
		ptsQueue.enqueue( pts + duration / 2.0 );
		durationQueue.enqueue( duration / 2.0 );
	}
	else {
		ptsQueue.enqueue( pts );
		durationQueue.enqueue( duration );
	}
	return true;
}



AVFrame* Yadif::pullFrame( double &pts, double &duration, double &ratio )
{
	int ret = av_buffersink_get_frame( bufferSinkCtx, filterFrame );
	if ( ret < 0 ) {
		/*char es[128];
		av_strerror( ret, es, 128 );
		qDebug() << "Error while pulling from filtergraph:" << es;*/
		return NULL;
	}
	pts = ptsQueue.dequeue();
	duration = durationQueue.dequeue();
	ratio = ar;
	return filterFrame;
}



bool Yadif::reset( bool sendFields, int videoStreamIndex, AVFormatContext *fmtCtx, AVCodecContext *codecCtx )
{
	if ( filterGraph ) {
		avfilter_graph_free( &filterGraph );
		filterGraph = NULL;
	}
	eof = false;
	ptsQueue.clear();
	durationQueue.clear();
	twice = sendFields;

	const char *filter_descr;
	if ( twice )
		filter_descr = "yadif=1";
	else
		filter_descr = "yadif=0";
	char args[512];
	int ret = 0;
	const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
	const AVFilter *buffersink = avfilter_get_by_name("buffersink");
	AVFilterInOut *outputs = avfilter_inout_alloc();
	AVFilterInOut *inputs  = avfilter_inout_alloc();
	AVRational time_base = fmtCtx->streams[videoStreamIndex]->time_base;
	enum AVPixelFormat pixFmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV422P, AV_PIX_FMT_NONE };

	filterGraph = avfilter_graph_alloc();
	if (!outputs || !inputs || !filterGraph) {
		if ( inputs )
			avfilter_inout_free( &inputs );
		if ( outputs )
			avfilter_inout_free( &outputs );
		return false;
	}

	/* buffer video source: the decoded frames from the decoder will be inserted here. */
	snprintf( args, sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
			time_base.num, time_base.den,
			codecCtx->sample_aspect_ratio.num, codecCtx->sample_aspect_ratio.den );

	ret = avfilter_graph_create_filter( &bufferSrcCtx, buffersrc, "in", args, NULL, filterGraph );
	if ( ret < 0 ) {
		qDebug() << "Cannot create buffer source";
		avfilter_inout_free( &inputs );
		avfilter_inout_free( &outputs );
		return false;
	}

	/* buffer video sink: to terminate the filter chain. */
	ret = avfilter_graph_create_filter( &bufferSinkCtx, buffersink, "out", NULL, NULL, filterGraph );
	if ( ret < 0 ) {
		qDebug() << "Cannot create buffer sink";
		avfilter_inout_free( &inputs );
		avfilter_inout_free( &outputs );
		return false;
	}

	ret = av_opt_set_int_list( bufferSinkCtx, "pix_fmts", pixFmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN );
	if ( ret < 0 ) {
		qDebug() << "Cannot set output pixel format";
		avfilter_inout_free( &inputs );
		avfilter_inout_free( &outputs );
		return false;
	}

	/*
	* Set the endpoints for the filter graph. The filter_graph will
	* be linked to the graph described by filters_descr.
	*/

	/*
	* The buffer source output must be connected to the input pad of
	* the first filter described by filters_descr; since the first
	* filter input label is not specified, it is set to "in" by
	* default.
	*/
	outputs->name       = av_strdup( "in" );
	outputs->filter_ctx = bufferSrcCtx;
	outputs->pad_idx    = 0;
	outputs->next       = NULL;

	/*
	* The buffer sink input must be connected to the output pad of
	* the last filter described by filters_descr; since the last
	* filter output label is not specified, it is set to "out" by
	* default.
	*/
	inputs->name       = av_strdup( "out" );
	inputs->filter_ctx = bufferSinkCtx;
	inputs->pad_idx    = 0;
	inputs->next       = NULL;

	if ( (ret = avfilter_graph_parse_ptr( filterGraph, filter_descr, &inputs, &outputs, NULL )) < 0 ) {
		avfilter_inout_free( &inputs );
		avfilter_inout_free( &outputs );
		return false;
	}

	if ( (ret = avfilter_graph_config( filterGraph, NULL )) < 0 ) {
		avfilter_inout_free( &inputs );
		avfilter_inout_free( &outputs );
		return false;
	}

	avfilter_inout_free( &inputs );
	avfilter_inout_free( &outputs );
	return true;
}
