// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;


#include "input/input_ff.h"



InputFF::InputFF() : InputBase(),
	decoder( new FFDecoder() ),
	semaphore( new QSemaphore( 1 ) ),
	running( false ),
	seekAndPlayPTS( 0 ),
	seekAndPlay( false )
{
	inputType = FFMPEG;

	for ( int i = 0; i < NUMINPUTFRAMES; ++i ) {
		freeAudioFrames.enqueue( new Frame( &freeAudioFrames ) );
		freeVideoFrames.enqueue( new Frame( &freeVideoFrames ) );
	}
}



InputFF::~InputFF()
{
	delete decoder;

	Frame *f;
	while ( (f = audioFrames.dequeue()) )
		delete f;
	while ( (f = freeAudioFrames.dequeue()) )
		delete f;
	while ( (f = videoFrames.dequeue()) )
		delete f;
	while ( (f = freeVideoFrames.dequeue()) )
		delete f;
}



void InputFF::flush()
{
	Frame *f;
	while ( (f = videoFrames.dequeue()) )
		f->release();
	while ( (f = audioFrames.dequeue()) )
		f->release();

	lastFrame.set( NULL );
	audioFrameList.reset( outProfile );
}




bool InputFF::open( QString fn )
{
	bool ok = decoder->open( fn );
	if ( ok )
		sourceName = fn;

	lastFrame.set( NULL );
	videoResampler.reset( outProfile.getVideoFrameDuration() );

	return ok;
}



bool InputFF::probe( QString fn, Profile *prof )
{
	return decoder->probe( fn, prof );
}



double InputFF::seekTo( double p )
{
	flush();
	mmiSeek();
	Frame *f = freeVideoFrames.dequeue();
	if ( !f )
		return p;
	AudioFrame *af = new AudioFrame( audioFrameList.getBytesPerSample() );
	decoder->seekTo( p, f, af );
	if ( af->buffer )
		audioFrameList.append( af );
	else
		delete af;
	if ( f->getBuffer() ) {
		lastFrame.set( f );
		videoResampler.reset( outProfile.getVideoFrameDuration() );
		videoResampler.outputPts = f->pts();
		resample( f );
		return f->pts();
	}
	else {
		f->release();
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
		if ( seekAndPlayPath != sourceName || outProfile.hasVideo() != decoder->haveVideo || outProfile.hasAudio() != decoder->haveAudio )
			open( seekAndPlayPath );
		seekTo( seekAndPlayPTS );
		seekAndPlay = false;
		running = true;
		semaphore->release();
	}

	// in case file has moved
	if ( !decoder->formatCtx ) {
		decoder->endOfFile = FFDecoder::EofPacket | FFDecoder::EofAudioPacket | FFDecoder::EofVideoPacket | FFDecoder::EofAudio | FFDecoder::EofVideoFrame | FFDecoder::EofVideo;
		running = false;
	}

	while ( running ) {
		doWait = 1;

		if ( decoder->haveVideo && !( decoder->endOfFile & FFDecoder::EofVideo ) ) {
			if ( (f = freeVideoFrames.dequeue()) ) {
				// resample if necessary
				if ( videoResampler.repeat && lastFrame.valid() ) {
					// duplicate previous frame
					lastFrame.get( f );
					videoResampler.duplicate( f );
					f->mmi = mmi;
					f->mmiProvider = mmiProvider;
					if ( !videoResampler.repeat )
						mmiIncrement();
					videoFrames.enqueue( f );
				}
				else {
					f->mmi = mmi;
					f->mmiProvider = mmiProvider;
					mmiIncrement();
					if ( decoder->decodeVideo( f ) ) {
						lastFrame.set( f );
						if ( !f->getBuffer() )
							qDebug() << "NO decode BUFFER";
						resample ( f );
					}
					else
						f->release();
				}

				if ( (decoder->endOfFile & FFDecoder::EofVideoFrame) && !videoResampler.repeat )
					decoder->endOfFile |= FFDecoder::EofVideo;
				doWait = 0;
			}
		}

		if ( decoder->haveAudio && !( decoder->endOfFile & FFDecoder::EofAudio ) ) {
			if ( audioFrameList.writable() ) {
				AudioFrame *af = new AudioFrame( audioFrameList.getBytesPerSample() );
				decoder->decodeAudio( af );
				if ( af->buffer )
					audioFrameList.append( af );
				doWait = 0;
			}
		}

		if ( decoder->haveVideo && decoder->haveAudio ) {
			if ( (decoder->endOfFile & FFDecoder::EofAudio) && (decoder->endOfFile & FFDecoder::EofVideo) ) {
				printf("ff.run break\n");
				break;
			}
		}
		else if ( decoder->haveVideo ) {
			if ( decoder->endOfFile & FFDecoder::EofVideo ) {
				printf("ff.run break\n");
				break;
			}
		}
		else if ( decoder->haveAudio ) {
			if ( decoder->endOfFile & FFDecoder::EofAudio ) {
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
	if ( !f->getBuffer() )
		qDebug() << "NO resample BUFFER";
	double delta = ( f->pts() + f->profile.getVideoFrameDuration() ) - ( videoResampler.outputPts + videoResampler.outputDuration );
	if ( outProfile.getVideoFrameRate() == inProfile.getVideoFrameRate() ) { // no resampling
		videoFrames.enqueue( f );
	}
	else if ( delta >= videoResampler.outputDuration ) {
		// this frame will be duplicated (delta / videoResampler.outputDuration) times
		printf("duplicate, delta=%f, f->pts=%f, outputPts=%f\n", delta, f->pts(), videoResampler.outputPts);
		// add some to delta to prevent subduplicate
		videoResampler.setRepeat( f->pts(), (delta + 1) / videoResampler.outputDuration );
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



Frame* InputFF::getVideoFrame()
{
	SemaphoreLocker sem( semaphore );

	if ( !decoder->haveVideo ) {
		return NULL;
	}

	while ( videoFrames.queueEmpty() ) {
		if ( decoder->endOfFile & FFDecoder::EofVideo ) {
			if ( lastFrame.valid() ) {
				while ( freeVideoFrames.queueEmpty() )
					usleep( 1000 );
				Frame *f = freeVideoFrames.dequeue();
				lastFrame.get( f );
				if ( !f->getBuffer() )
					qDebug() << "NO LAST BUFFER";
				return f;
			}
			return NULL;
		}
		usleep( 1000 );
	}

	Frame *f = videoFrames.dequeue();
	if ( !f->getBuffer() )
		qDebug() << "NO BUFFER";
	return f;
}



Frame* InputFF::getAudioFrame( int nSamples )
{
	SemaphoreLocker sem( semaphore );

	if ( !decoder->haveAudio ) {
		return NULL;
	}

	while ( freeAudioFrames.queueEmpty() ) {
		usleep( 1000 );
	}
	Frame *f = freeAudioFrames.dequeue();

	while ( !audioFrameList.readable( nSamples ) ) {
		if ( decoder->endOfFile & FFDecoder::EofAudio ) {
			f->setAudioFrame( outProfile.getAudioChannels(), outProfile.getAudioSampleRate(), Profile::bytesPerChannel( &outProfile ), nSamples, audioFrameList.readPts() );
			int n = audioFrameList.read( f->data() );
			if ( !n ) {
				f->release();
				return NULL;
			}
			// complete with silence
			memset( f->data() + ( n * audioFrameList.getBytesPerSample() ), 0, (nSamples - n) * audioFrameList.getBytesPerSample() );
			return f;
		}
		usleep( 1000 );
	}

	f->setAudioFrame( outProfile.getAudioChannels(), outProfile.getAudioSampleRate(), Profile::bytesPerChannel( &outProfile ), nSamples, audioFrameList.readPts() );
	audioFrameList.read( f->data(), nSamples );

	return f;
}
