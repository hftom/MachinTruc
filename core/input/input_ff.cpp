// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include <sys/time.h>

#include <QTimer>

#include "input/input_ff.h"

#define DECODEAUDIOSYNC 1
#define DECODEAUDIOPROBE 2



int lockManagerRegistered = 0;

int lockManager( void **mutex, enum AVLockOp op)
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

static const int NCFR = 9;
static const double CommonFrameRates[NCFR][2] = {
	{ 24000., 1001. },
	{ 24., 1. },
	{ 25000., 1001. },
	{ 25., 1. },
	{ 30000., 1001. },
	{ 30., 1. },
	{ 50., 1. },
	{ 60000., 1001. },
	{ 60., 1. },
};



InputFF::InputFF() : InputBase(),
	formatCtx( NULL ),
	videoCodecCtx( NULL ),
	audioCodecCtx( NULL ),
	swr( NULL ),
	videoCodec( NULL ),
	audioCodec( NULL ),
	videoStream( -1 ),
	audioStream( -1 ),
	duration( 0 ),
	startTime( 0 ),
	seekAndPlay( false ),
	arb( NULL ),
	endOfFile( 0 ),
	running( false ),
	oneShot( false )
{
	inputType = FFMPEG;

	if ( !lockManagerRegistered ) {
		av_lockmgr_register( lockManager );
		lockManagerRegistered = 1;
		avcodec_register_all();
		av_register_all();
	}

	videoAvframe = av_frame_alloc();
	audioAvframe = av_frame_alloc();

	int i;
	for ( i = 0; i < NUMINPUTFRAMES; ++i ) {
		freeAudioFrames.enqueue( new Frame( &freeAudioFrames ) );
		freeVideoFrames.enqueue( new Frame( &freeVideoFrames ) );
	}

	semaphore = new QSemaphore( 1 );
}



InputFF::~InputFF()
{
	close();

	if ( videoAvframe )
		av_free( videoAvframe );

	if ( audioAvframe )
		av_free( audioAvframe );

	Frame *f;

	while ( (f = audioFrames.dequeue()) )
		delete f;
	while ( (f = freeAudioFrames.dequeue()) )
		delete f;

	while ( (f = videoFrames.dequeue()) )
		delete f;
	while ( (f = freeVideoFrames.dequeue()) )
		delete f;

	if ( arb )
		delete arb;
}



void InputFF::close()
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



bool InputFF::open( QString fn )
{
	close();
	if ( !ffOpen( fn ) )
		return false;

	duration = formatCtx->duration;
	startTime = (formatCtx->start_time == AV_NOPTS_VALUE) ? 0 : formatCtx->start_time;
	sourceName = fn;

	return true;
}


// Critical function. We MUST get the right values.
// Returns true if this file is supported.
bool InputFF::probe( QString fn, Profile *prof )
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
		// seek to 10% for thumbnail
		double seekTarget = startTime + (duration * 10.0 / 100.0);
		flush();
		if ( !seek( seekTarget ) ) {
			return haveAudio;
		}

		Frame *ff;
		if ( !(ff = freeVideoFrames.dequeue()) ) {
			return haveAudio;
		}
		if ( !seekDecodeNext( ff ) ) {
			ff->release();
			return haveAudio;
		}
		// enqueue this frame for thumbnail
		videoFrames.enqueue( ff );
		// check for supported formats
		if ( ff->type() != Frame::YUV420P && ff->type() != Frame::YUV422P ) {
			return haveAudio;
		}
		// decode some frames
		// check for interlaced
		// and get the average frame rate
		bool ilaced = false;
		bool tff = true;
		double firstpts, lastpts;
		int nframes = 0;
		Frame f( NULL );
		lastpts = firstpts = ff->pts();
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
			if ( !decodeAudio( DECODEAUDIOPROBE, &pts ) ) {
				return haveAudio;
			}
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



bool InputFF::ffOpen( QString fn )
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

	for ( i = 0; i < formatCtx->nb_streams; i++ ) {
		if ( videoStream == -1 && outProfile.hasVideo() && formatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO ) {
			if ( (videoCodecCtx = formatCtx->streams[ i ]->codec) ) {
				if ( (videoCodec = avcodec_find_decoder( videoCodecCtx->codec_id )) ) {
					videoStream = i;
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
		videoResampler.reset( outProfile.getVideoFrameDuration() );
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



AVSampleFormat InputFF::convertProfileSampleFormat( int f )
{
	if ( f == Profile::SAMPLE_FMT_32F )
		return AV_SAMPLE_FMT_FLT;

	return AV_SAMPLE_FMT_S16;
}



qint64 InputFF::convertProfileAudioLayout( int layout )
{
	if ( layout == Profile::LAYOUT_51 )
		return AV_CH_LAYOUT_5POINT1 ;

	return AV_CH_LAYOUT_STEREO;
}



void InputFF::resetAudioResampler( bool resetArb )
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

	if ( !arb )
		arb = new AudioRingBuffer();
	if ( resetArb ) {
		arb->reset();
		arb->setBytesPerChannel( Profile::bytesPerChannel( &outProfile ) );
		arb->setChannels( outProfile.getAudioChannels() );
		arb->setSampleRate( outProfile.getAudioSampleRate() );
	}
}



void InputFF::flush()
{
	Frame *f;
	while ( (f = videoFrames.dequeue()) )
		f->release();
	while ( (f = audioFrames.dequeue()) )
		f->release();

	resetAudioResampler();
}



bool InputFF::seek( double t )
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

		videoResampler.reset( outProfile.getVideoFrameDuration() );

		while ( !videoPackets.isEmpty() )
			freePacket( videoPackets.takeFirst() );
		while ( !audioPackets.isEmpty() )
			freePacket( audioPackets.takeFirst() );
	}

	endOfFile = 0;
	return true;
}



void InputFF::seekFast( float percent )
{
	double seekTarget = startTime + (duration * percent / 100.0);
	printf("seeking : %f - %f - %f\n", percent, seekTarget, duration );
	flush();
	if ( !seek( seekTarget ) )
		return;
	seekNext();
}



void InputFF::seekNext()
{
	Frame *f;
	if ( (f = freeVideoFrames.dequeue()) ) {
		if ( seekDecodeNext( f ) )
			videoFrames.enqueue( f );
		else
			f->release();
	}
}



bool InputFF::seekDecodeNext( Frame *f )
{
	oneShot = true;
	bool ret = decodeVideo( f );
	oneShot = false;
	return ret;
}



double InputFF::seekTo( double p )
{
	mmiSeek();

	if ( !formatCtx || ( p < startTime ) )
		return p;

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
				if ( !decodeAudio( DECODEAUDIOSYNC, &p ) ) {
					printf("! decodeAudio\n");
					timestamp -= MICROSECOND;
				}
				else
					break;
			}
		}
	}
	else {
		Frame *f = freeVideoFrames.dequeue();
		if ( !f ) {
			return p;
		}

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
				f->release();
				return p;
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
				else if ( haveAudio && !decodeAudio( DECODEAUDIOSYNC, &cur ) )
					timestamp -= seekinc * hdur;
				else
					break;
			}
			else if ( delta > hdur ) {
				do {
					if ( !seekDecodeNext( f ) ) {
						f->release();
						return p;
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
				else if ( haveAudio && !decodeAudio( DECODEAUDIOSYNC, &cur ) )
					timestamp -= seekinc * hdur;
				else
					break;
			}
			else if ( haveAudio && !decodeAudio( DECODEAUDIOSYNC, &cur ) )
				timestamp -= seekinc * hdur;
			else
				break;

			lastpts = cur;
		}
		p = videoResampler.outputPts = f->pts();
		resample( f );
	}

	return p;
}



/* openSeekPlay is asynchronous and as such it could be called again before open and seek
 * have completed. So the need for a mutex that is also required in getVideoFrame/getAudioFrame
 * since we want to wait for seek completion before asking for a frame.
 * We use a semaphore here because a mutex must be locked and unlocked in the same thread
 * but we have to lock in openSeekPlay (main thread) and unlock in run (other thread).
 * This SemaphoreLocker used in getVideoFrame/getAudioFrame behaves like QMutexLocker.*/
class SemaphoreLocker
{
public:
	explicit SemaphoreLocker( QSemaphore *s ) {
		sem = s;
		sem->acquire();
	}
	~SemaphoreLocker() {
		sem->release();
	}

private:
	QSemaphore *sem;
};



void InputFF::openSeekPlay( QString fn, double p )
{
	semaphore->acquire();
	play( false );
	seekAndPlayPTS = p;
	seekAndPlayPath = fn;
	seekAndPlay = true;
	endOfFile = 0;
	start();
}



void InputFF::play( bool b )
{
	if ( !b ) {
		running = false;
		wait();
	}
	else {
		running = true;
		start();
	}
}



void InputFF::run()
{
	int doWait;
	Frame *f;

	if ( seekAndPlay ) {
		if ( seekAndPlayPath != sourceName || outProfile.hasVideo() != haveVideo || outProfile.hasAudio() != haveAudio )
			open( seekAndPlayPath );
		seekTo( seekAndPlayPTS );
		seekAndPlay = false;
		running = true;
		semaphore->release();
	}

	while ( running ) {
		doWait = 1;

		if ( haveVideo && !( endOfFile & EofVideo ) ) {
			if ( (f = freeVideoFrames.dequeue()) ) {
				// resample if necessary
				if ( videoResampler.repeat && videoResampler.repeatBuffer ) {
					// duplicate previous frame
					videoResampler.duplicate( f );
					f->mmi = mmi;
					if ( !videoResampler.repeat )
						mmiIncrement();
					videoFrames.enqueue( f );
				}
				else {
					f->mmi = mmi;
					mmiIncrement();
					if ( decodeVideo( f ) ) {
						resample ( f );
					}
					else
						f->release();
				}
				if ( (endOfFile & EofVideoFrame) && !videoResampler.repeat )
					endOfFile |= EofVideo;
				doWait = 0;
			}
		}

		if ( haveAudio && !( endOfFile & EofAudio ) ) {
			if ( arb->writable() ) {
				decodeAudio();
				doWait = 0;
			}
		}

		if ( haveVideo && haveAudio ) {
			if ( (endOfFile & EofAudio) && (endOfFile & EofVideo) ) {
				printf("ff.run break\n");
				break;
			}
		}
		else if ( haveVideo ) {
			if ( endOfFile & EofVideo ) {
				printf("ff.run break\n");
				break;
			}
		}
		else if ( haveAudio ) {
			if ( endOfFile & EofAudio ) {
				printf("ff.run break\n");
				break;
			}
		}

		if ( doWait ) {
			usleep( 1000 );
		}
	}
}



void InputFF::resample( Frame *f )
{
	double delta = ( f->pts() + f->profile.getVideoFrameDuration() ) - ( videoResampler.outputPts + videoResampler.outputDuration );
	if ( outProfile.getVideoFrameRate() == inProfile.getVideoFrameRate() ) { // no resampling
		videoFrames.enqueue( f );
	}
	else if ( delta >= videoResampler.outputDuration ) {
		// this frame will be duplicated (delta / videoResampler.outputDuration) times
		printf("duplicate, delta=%f, f->pts=%f, outputPts=%f\n", delta, f->pts(), videoResampler.outputPts);
		videoResampler.setRepeat( f, delta / videoResampler.outputDuration );
		videoFrames.enqueue( f );
		videoResampler.outputPts += videoResampler.outputDuration;
	}
	else if ( delta <= -videoResampler.outputDuration ) {
		// skip
		printf("skip frame delta=%f, f->pts=%f, outputPts=%f\n", delta, f->pts(), videoResampler.outputPts);
		f->release();
	}
	else {
		videoFrames.enqueue( f );
		videoResampler.outputPts += videoResampler.outputDuration;
	}
}



bool InputFF::decodeVideo( Frame *f )
{
	int gotFrame = 0;
	double vpts = 0;

	while ( running || oneShot ) {
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
			endOfFile |= EofVideoFrame;
			return false;
		}
	}
	return false;
}



bool InputFF::makeFrame( Frame *f, double ratio, double pts, double dur )
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
			f->profile.setVideoColorSpace( (height > 576) ? Profile::SPC_709 : Profile::SPC_601_625 );
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
				ratio, videoAvframe->interlaced_frame, videoAvframe->top_field_first, pts, dur );
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
				ratio, videoAvframe->interlaced_frame, videoAvframe->top_field_first, pts, dur );
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



bool InputFF::decodeAudio( int sync, double *pts )
{
	int gotFrame = 0;
	int writtenSamples = 0;

	while ( (sync) ? 1 : running ) {
		if ( !currentAudioPacket.packet ) {
			while ( audioPackets.isEmpty() ) {
				if ( !getPacket() ) {
					// get the last few samples
					int n = swr_get_delay( swr, arb->getSampleRate() ) * arb->getChannels();
					uint8_t* dst = arb->write( writtenSamples, n );
					int outSamples = swr_convert( swr, &dst, n, 0, 0 );
					writtenSamples += outSamples;
					if ( writtenSamples )
						arb->writeDone( 0, writtenSamples );
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
					vpts -= swr_get_delay( swr, arb->getSampleRate() ) * MICROSECOND / arb->getSampleRate();
				}

				// roundup estimate
				int outSamples = av_rescale_rnd( swr_get_delay( swr, audioCodecCtx->sample_rate ) + audioAvframe->nb_samples, arb->getSampleRate(), audioCodecCtx->sample_rate, AV_ROUND_UP );
				// FIXME:nasty hack to avoid ringbuffer overflow
				outSamples *= arb->getChannels();

				// audio channels and layout may have changed
				// if so, we flush swr internal buffer and reset
				int64_t layout = (audioCodecCtx->channel_layout && audioCodecCtx->channels == av_get_channel_layout_nb_channels(audioCodecCtx->channel_layout)) ?
								audioCodecCtx->channel_layout : av_get_default_channel_layout(audioCodecCtx->channels);
				if ( audioCodecCtx->channels != lastChannels || layout != lastLayout ) {
					//printf("channels=%d, lastChannels=%d\n", audioCodecCtx->channels, lastChannels);
					int n = swr_get_delay( swr, arb->getSampleRate() ) * arb->getChannels();
					uint8_t* dst = arb->write( writtenSamples, n );
					int outSamples = swr_convert( swr, &dst, n, 0, 0 );
					writtenSamples += outSamples;
					resetAudioResampler( false );
				}

				// We have to convert in order to get the exact number of out samples
				// and swr_convert writes directly into the ringbuffer.
				// So, if all samples have to be skipped, don't call arb->writeDone,
				// thus the write pointer isn't increased and next time we will overwrite this chunk.
				uint8_t* dst = arb->write( writtenSamples, outSamples );
				outSamples = swr_convert( swr, &dst, outSamples, (const uint8_t**)&audioAvframe->data[0], audioAvframe->nb_samples );
				writtenSamples += outSamples;
				if ( sync ) {
					if ( sync == DECODEAUDIOPROBE ) {
						*pts = vpts;
						return true;
					}
					// how much samples to skip?
					int ns = (*pts - vpts) * (double)arb->getSampleRate() / MICROSECOND;
					printf("pts=%f, vpts=%f, ns=%d, sr=%d, outSamples=%d\n", *pts, vpts, ns, arb->getSampleRate(), outSamples);
					if ( ns < 0 ) {
						// we have to seek back again
						freeCurrentAudioPacket();
						return false;
					}
					else if ( ns < outSamples ) {
						// Tell the ringbuffer that "ns" samples are skipped
						arb->writeDone( *pts, writtenSamples, ns );
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
					arb->writeDone( vpts, writtenSamples );
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



void InputFF::shiftCurrentAudioPacketPts( double pts )
{
	currentAudioPacket.pts = pts + (MICROSECOND * audioAvframe->nb_samples / audioCodecCtx->sample_rate);
}



void InputFF::shiftCurrentAudioPacket( int len )
{
	currentAudioPacket.packet->size -= len;
	currentAudioPacket.packet->data += len;
}



void InputFF::freeCurrentAudioPacket()
{
	currentAudioPacket.reset();
	freePacket( currentAudioPacket.packet );
	currentAudioPacket.free();
}



bool InputFF::getPacket()
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



void InputFF::freePacket( AVPacket *packet )
{
	av_free_packet( packet );
	free( packet );
}



Frame* InputFF::getVideoFrame()
{
	SemaphoreLocker sem( semaphore );

	if ( !haveVideo ) {
		return NULL;
	}

	while ( videoFrames.queueEmpty() ) {
		if ( endOfFile & EofVideo ) {
			return NULL;
		}
		usleep( 1000 );
	}

	Frame *f = videoFrames.dequeue();
	return f;
}



Frame* InputFF::getAudioFrame( int nSamples )
{
	SemaphoreLocker sem( semaphore );

	if ( !haveAudio ) {
		return NULL;
	}

	while ( freeAudioFrames.queueEmpty() ) {
		usleep( 1000 );
	}
	Frame *f = freeAudioFrames.dequeue();

	while ( !arb->readable( nSamples ) ) {
		if ( endOfFile & EofAudio ) {
			f->setAudioFrame( arb->getChannels(), arb->getSampleRate(), arb->getBytesPerChannel(), nSamples, arb->readPts() );
			int n = arb->read( f->data() );
			if ( !n ) {
				f->release();
				return NULL;
			}
			// complete with silence
			memset( f->data() + ( n * arb->getBytesPerSample() ), 0, (nSamples - n) * arb->getBytesPerSample() );
			return f;
		}
		usleep( 1000 );
	}

	f->setAudioFrame( arb->getChannels(), arb->getSampleRate(), arb->getBytesPerChannel(), nSamples, arb->readPts() );
	arb->read( f->data(), nSamples );

	return f;
}



////////////////////////////////////////////////////////////////////
AudioRingBuffer::AudioRingBuffer()
	: reader(0),
	writer(0),
	size(25),
	channels(DEFAULTCHANNELS),
	bytesPerChannel(2),
	bytesPerSample(channels * bytesPerChannel),
	sampleRate(DEFAULTSAMPLERATE)
{
	chunks = new AudioChunk[size];
}



AudioRingBuffer::~AudioRingBuffer()
{
	delete [] chunks;
}



void AudioRingBuffer::reset()
{
	reader = writer = 0;
}



int AudioRingBuffer::getBytesPerSample()
{
	return bytesPerSample;
}



void AudioRingBuffer::setBytesPerChannel( int n )
{
	bytesPerChannel = n;
	bytesPerSample = channels * bytesPerChannel;
}

int AudioRingBuffer::getBytesPerChannel()
{
	return bytesPerChannel;
}



void AudioRingBuffer::setChannels( int c )
{
	channels = c;
	bytesPerSample = channels * bytesPerChannel;
}

int AudioRingBuffer::getChannels()
{
	return channels;
}



void AudioRingBuffer::setSampleRate( int r )
{
	sampleRate = r;
}

int AudioRingBuffer::getSampleRate()
{
	return sampleRate;
}



double AudioRingBuffer::readPts()
{
	return chunks[reader].pts;
}



bool AudioRingBuffer::readable( int nSamples )
{
	QMutexLocker ml( &mutex );
	int i = reader;
	int n = 0;
	while ( i != writer ) {
		n += chunks[i].available;
		if ( n >= nSamples )
			return true;
		if ( ++i >= size )
			i = 0;
	}
	return false;
}



void AudioRingBuffer::read( uint8_t *dst, int nSamples )
{
	QMutexLocker ml( &mutex );
	uint8_t *d = dst;
	while ( nSamples > 0 ) {
		bool shift = false;
		int ns = chunks[reader].available;
		if ( ns > nSamples ) {
			ns = nSamples;
			shift = true;
		}
		int blen = ns * bytesPerSample;
		memcpy( d, chunks[reader].bytes + chunks[reader].bufOffset, blen );
		if ( shift ) {
			chunks[reader].bufOffset += blen;
			chunks[reader].available -= ns;
			chunks[reader].pts += (double)ns * MICROSECOND / sampleRate;
			return;
		}
		nSamples -= ns;
		d += blen;
		if ( ++reader >= size )
			reader = 0;
	}
}



int AudioRingBuffer::read( uint8_t *dst )
{
	QMutexLocker ml( &mutex );
	uint8_t *d = dst;
	int nSamples = 0;
	while ( reader != writer ) {
		int ns = chunks[reader].available;
		int blen = ns * bytesPerSample;
		memcpy( d, chunks[reader].bytes + chunks[reader].bufOffset, blen );
		nSamples += ns;
		d += blen;
		if ( ++reader >= size )
			reader = 0;
	}
	return nSamples;
}



bool AudioRingBuffer::writable()
{
	QMutexLocker ml( &mutex );
	if ( writer == reader ) // true when ringbuffer is empty
		return true;
	int dist = 0;
	if ( writer < reader )
		dist = reader - writer;
	else
		dist = size - 1 - writer + reader;

	return ( dist > 1 );
}



uint8_t* AudioRingBuffer::write( int writtenSamples, int moreSamples )
{
	QMutexLocker ml( &mutex );
	if ( chunks[writer].bufSize < ((writtenSamples + moreSamples) * bytesPerSample) ) {
		chunks[writer].bufSize = (writtenSamples + moreSamples) * bytesPerSample;
		if ( !chunks[writer].bytes )
			chunks[writer].bytes = (uint8_t*)malloc( chunks[writer].bufSize );
		else
			chunks[writer].bytes = (uint8_t*)realloc( chunks[writer].bytes, chunks[writer].bufSize );
	}

	return chunks[writer].bytes + (writtenSamples * bytesPerSample);
}



void AudioRingBuffer::writeDone( double pts, int nSamples, int samplesOffset )
{
	QMutexLocker ml( &mutex );
	chunks[writer].pts = pts;
	chunks[writer].available = nSamples - samplesOffset;
	chunks[writer].bufOffset = samplesOffset * bytesPerSample;
	if ( ++writer >= size )
		writer = 0;
}
