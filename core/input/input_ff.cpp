// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;


#include "input/input_ff.h"

#define BACKWARDLEN MICROSECOND



InputFF::InputFF() : InputBase(),
	decoder( new FFDecoder() ),
	semaphore( new QSemaphore( 1 ) ),
	running( false ),
	seekAndPlayPTS( 0 ),
	seekAndPlay( false ),
	backwardAudioSamples( 0 ),
	playBackward( false ),
	backwardPts( 0 ),
	backwardStartPts( 0 ),
	backwardEof( false ),
	eofVideo( false ),
	eofAudio( false )
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

	flush();

	Frame *f;
	while ( (f = freeAudioFrames.dequeue()) )
		delete f;
	while ( (f = freeVideoFrames.dequeue()) )
		delete f;
}



void InputFF::flush()
{
	Frame *f;
	while ( (f = videoFrames.dequeue()) )
		f->release();
	while ( (f = reorderedVideoFrames.dequeue()) )
		delete f;
	while ( !backwardVideoFrames.isEmpty() )
		delete backwardVideoFrames.takeFirst();

	while ( (f = audioFrames.dequeue()) )
		f->release();
	while ( !backwardAudioFrames.isEmpty() )
		delete backwardAudioFrames.takeFirst();

	lastFrame.set( NULL );
	audioFrameList.reset( outProfile );
	videoResampler.reset( outProfile.getVideoFrameDuration() );
	eofVideo = eofAudio = false;
	backwardEof = false;
	backwardAudioSamples = 0;
}




bool InputFF::open( QString fn )
{
	bool ok = decoder->open( fn );
	if ( ok )
		sourceName = fn;

	flush();

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

	if ( playBackward ) {
		p = qMin( p, inProfile.getStreamStartTime() + inProfile.getStreamDuration() - inProfile.getVideoFrameDuration() );
		backwardPts = backwardStartPts = p;
		double target = backwardPts - BACKWARDLEN;
		if ( target <= inProfile.getStreamStartTime() ) {
			target = inProfile.getStreamStartTime();
			backwardEof = true;
		}
		Frame *f = new Frame( NULL );
		AudioFrame *af = new AudioFrame( audioFrameList.getBytesPerSample(), outProfile.getAudioSampleRate() );
		bool ok = decoder->seekTo( target, f, af );
		if ( af->buffer ) {
			backwardAudioFrames.append( af );
			backwardAudioSamples += af->available;
		}
		else
			delete af;
		if ( ok && f->getBuffer() ) {
			backwardVideoFrames.append( f );
			videoResampler.reset( outProfile.getVideoFrameDuration() );
			videoResampler.outputPts = p;
			return f->pts();
		}
		else {
			delete f;
		}
	}
	else {
		Frame *f = freeVideoFrames.dequeue();
		if ( !f )
			return p;
		AudioFrame *af = new AudioFrame( audioFrameList.getBytesPerSample(), outProfile.getAudioSampleRate() );
		bool ok = decoder->seekTo( p, f, af );
		if ( af->buffer )
			audioFrameList.append( af );
		else
			delete af;
		if ( ok && f->getBuffer() ) {
			lastFrame.set( f );
			videoResampler.reset( outProfile.getVideoFrameDuration() );
			videoResampler.outputPts = f->pts();
			resample( f );
			return f->pts();
		}
		else {
			f->release();
		}
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



void InputFF::openSeekPlay( QString fn, double p, bool backward )
{
	semaphore->acquire();
	play( false );
	seekAndPlayPTS = p;
	seekAndPlayPath = fn;
	seekAndPlay = true;
	playBackward = backward;
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
		decoder->endOfFile = FFDecoder::EofPacket | FFDecoder::EofAudioPacket | FFDecoder::EofVideoPacket | FFDecoder::EofAudio | FFDecoder::EofVideo;
		running = false;
	}

	if ( playBackward )
		runBackward();
	else
		runForward();
}



void InputFF::runForward()
{
	int doWait;
	Frame *f;

	while ( running ) {
		doWait = 1;

		if ( decoder->haveVideo && !eofVideo ) {
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
						resample ( f );
					}
					else
						f->release();
				}

				if ( (decoder->endOfFile & FFDecoder::EofVideo) && !videoResampler.repeat )
					eofVideo = true;
				doWait = 0;
			}
		}

		if ( decoder->haveAudio && !eofAudio ) {
			if ( audioFrameList.writable() ) {
				AudioFrame *af = new AudioFrame( audioFrameList.getBytesPerSample(), outProfile.getAudioSampleRate() );
				decoder->decodeAudio( af );
				if ( af->buffer )
					audioFrameList.append( af );
				else
					delete af;

				if ( decoder->endOfFile & FFDecoder::EofAudio )
					eofAudio = true;
				doWait = 0;
			}
		}

		if ( decoder->haveVideo && decoder->haveAudio ) {
			if ( eofVideo && eofAudio ) {
				printf("ff.run break\n");
				break;
			}
		}
		else if ( decoder->haveVideo ) {
			if ( eofVideo ) {
				printf("ff.run break\n");
				break;
			}
		}
		else if ( decoder->haveAudio ) {
			if ( eofAudio ) {
				printf("ff.run break\n");
				break;
			}
		}

		if ( doWait ) {
			usleep( 1000 );
		}
	}
}



void InputFF::runBackward()
{
	int doWait;
	bool endVideoSequence = !decoder->haveVideo;
	bool endAudioSequence = !decoder->haveAudio;
	int minSamples = (outProfile.getAudioSampleRate() / outProfile.getVideoFrameRate()) * (NUMINPUTFRAMES + 1);

	while ( running ) {
		doWait = 1;

		if ( !endVideoSequence ) {
			bool yes = backwardVideoFrames.count() + reorderedVideoFrames.count() < (inProfile.getVideoFrameRate() * BACKWARDLEN / MICROSECOND) + 3;
			yes |= reorderedVideoFrames.count() < 3;
			yes |= decoder->haveAudio && !audioFrameList.readable( minSamples );
			if ( yes ) {
				Frame *f = new Frame( NULL );
				if ( decoder->decodeVideo( f ) ) {
					backwardVideoFrames.append( f );
					if ( f->pts() >= backwardPts )
						endVideoSequence = true;
				}
				else {
					delete f;
					endVideoSequence = true;
				}
				doWait = 0;
			}
		}

		if ( !endAudioSequence ) {
			bool yes = !audioFrameList.readable( minSamples * 3 );
			yes |= decoder->haveVideo && reorderedVideoFrames.count() < 3;
			if ( yes ) {
				AudioFrame *af = new AudioFrame( audioFrameList.getBytesPerSample(), outProfile.getAudioSampleRate() );
				decoder->decodeAudio( af );
				if ( af->available ) {
					double dpts = (double)(backwardAudioSamples + af->available) * MICROSECOND / (double)outProfile.getAudioSampleRate();
					if ( backwardAudioFrames.count() && backwardAudioFrames.first()->bufPts + dpts >= backwardStartPts ) {
						af->available -= ((backwardAudioFrames.first()->bufPts + dpts - backwardStartPts) * (double)outProfile.getAudioSampleRate() / MICROSECOND) - 0.5;
						if ( af->available <= 0 )
							delete af;
						else {
							backwardAudioSamples += af->available;
							backwardAudioFrames.append( af );
						}
						endAudioSequence = true;
					}
					else {
						backwardAudioSamples += af->available;
						backwardAudioFrames.append( af );
					}
				}
				else {
					delete af;
					endAudioSequence = true;
				}
				doWait = 0;
			}
		}

		if ( endVideoSequence && endAudioSequence ) {
			if ( backwardVideoFrames.count() ) {
				backwardPts = backwardVideoFrames.first()->pts();
				if ( backwardVideoFrames.first()->pts() <= inProfile.getStreamStartTime() )
					backwardEof = true;
			}
			else if ( backwardAudioFrames.count() ) {
				backwardPts = backwardAudioFrames.first()->bufPts;
				if ( backwardAudioFrames.first()->bufPts <= inProfile.getStreamStartTime() )
					backwardEof = true;
			}
			else
				backwardEof = true;

			double bpts = backwardPts;
			while ( !backwardVideoFrames.isEmpty() ) {
				// resample if necessary
				if ( videoResampler.repeat && lastFrame.valid() ) {
					// duplicate previous frame
					Frame *f = new Frame( NULL );
					lastFrame.get( f, true );
					videoResampler.duplicate( f, true );
					f->mmi = mmi;
					f->mmiProvider = mmiProvider;
					if ( !videoResampler.repeat )
						mmiIncrement();
					reorderedVideoFrames.enqueue( f );
				}
				else {
					Frame *f = backwardVideoFrames.takeLast();
					f->mmi = mmi;
					f->mmiProvider = mmiProvider;
					mmiIncrement();
					lastFrame.set( f );
					resampleBackward( f );
				}
				bpts = videoResampler.outputPts + videoResampler.outputDuration;
			}
			backwardPts = bpts;

			while ( !backwardAudioFrames.isEmpty() ) {
				AudioFrame *af = backwardAudioFrames.takeLast();
				if ( !af->available ) {
					delete af;
					continue;
				}
				int bps = audioFrameList.getBytesPerSample();
				int size = af->available * bps;
				Buffer *buffer = BufferPool::globalInstance()->getBuffer( size );
				uint8_t *src = af->buffer->data() + af->bufOffset + size - bps;
				uint8_t *dst = buffer->data();
				af->bufSize = size;
				af->bufOffset = 0;
				while ( size ) {
					memcpy( dst, src, bps );
					src -= bps;
					dst += bps;
					size -= bps;
				}
				BufferPool::globalInstance()->releaseBuffer( af->buffer );
				af->buffer = buffer;

				audioFrameList.append( af );
			}

			if ( backwardEof ) {
				eofVideo = eofAudio = true;
				printf("ff.run break\n");
				break;
			}
			else {
				double target = backwardPts - BACKWARDLEN;
				if ( target <= inProfile.getStreamStartTime() ) {
					target = inProfile.getStreamStartTime();
					backwardEof = true;
				}
				Frame *f = new Frame( NULL );
				AudioFrame *af = new AudioFrame( audioFrameList.getBytesPerSample(), outProfile.getAudioSampleRate() );
				bool ok = decoder->seekTo( target, f, af );
				if ( af->buffer ) {
					backwardAudioSamples += af->available;
					if ( !decoder->haveVideo && af->bufPts == 0 )
						af->bufPts = target;
					backwardAudioFrames.append( af );
				}
				else
					delete af;
				if ( ok && f->getBuffer() )
					backwardVideoFrames.append( f );
				else
					delete f;
			}
			endVideoSequence = !decoder->haveVideo;
			endAudioSequence = !decoder->haveAudio;
		}

		if ( doWait ) {
			usleep( 1000 );
		}
	}
}



void InputFF::resample( Frame *f )
{
	double delta = ( f->pts() + f->profile.getVideoFrameDuration() ) - ( videoResampler.outputPts + videoResampler.outputDuration );
	/*if ( outProfile.getVideoFrameRate() == inProfile.getVideoFrameRate() ) { // no resampling
		videoFrames.enqueue( f );
	}
	else*/ if ( delta >= videoResampler.outputDuration ) {
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



void InputFF::resampleBackward( Frame *f )
{
	double delta = ( videoResampler.outputPts - videoResampler.outputDuration ) - ( f->pts() - f->profile.getVideoFrameDuration() );
	if ( delta >= videoResampler.outputDuration ) {
		// this frame will be duplicated (delta / videoResampler.outputDuration) times
		printf("duplicate, delta=%f, f->pts=%f, outputPts=%f\n", delta, f->pts(), videoResampler.outputPts);
		// add some to delta to prevent subduplicate
		videoResampler.setRepeat( f->pts(), (delta + 1) / videoResampler.outputDuration );
		reorderedVideoFrames.enqueue( f );
		videoResampler.outputPts -= videoResampler.outputDuration;
	}
	else if ( delta <= -videoResampler.outputDuration ) {
		// skip
		printf("skip frame delta=%f, f->pts=%f, outputPts=%f\n", delta, f->pts(), videoResampler.outputPts);
		delete f;
	}
	else {
		reorderedVideoFrames.enqueue( f );
		videoResampler.outputPts -= videoResampler.outputDuration;
	}
}



Frame* InputFF::getVideoFrame()
{
	SemaphoreLocker sem( semaphore );

	if ( !decoder->haveVideo ) {
		return NULL;
	}

	if ( playBackward ) {
		while ( freeVideoFrames.queueEmpty() )
			usleep( 1000 );
		while ( reorderedVideoFrames.queueEmpty() ) {
			if ( eofVideo )
				return NULL;
			//qDebug() << "wait video";
			usleep( 1000 );
		}
		Frame *rf = reorderedVideoFrames.dequeue();
		Frame *f = freeVideoFrames.dequeue();
		f->setSharedBuffer( rf->getBuffer() );
		f->setVideoFrame( (Frame::DataType)rf->type(), rf->profile.getVideoWidth(), rf->profile.getVideoHeight(),
						  rf->profile.getVideoSAR(), rf->profile.getVideoInterlaced(),
						  rf->profile.getVideoTopFieldFirst(), rf->pts(),
						  rf->profile.getVideoFrameDuration(),
						  rf->orientation() );
		f->profile = rf->profile;
		f->mmi = rf->mmi;
		f->mmiProvider = rf->mmiProvider;
		delete rf;
		return f;
	}

	while ( videoFrames.queueEmpty() ) {
		if ( eofVideo ) {
			if ( lastFrame.valid() ) {
				while ( freeVideoFrames.queueEmpty() )
					usleep( 1000 );
				Frame *f = freeVideoFrames.dequeue();
				lastFrame.get( f );
				return f;
			}
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

	if ( !decoder->haveAudio ) {
		return NULL;
	}

	while ( freeAudioFrames.queueEmpty() ) {
		usleep( 1000 );
	}
	Frame *f = freeAudioFrames.dequeue();

	while ( !audioFrameList.readable( nSamples ) ) {
		if ( eofAudio ) {
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
		//qDebug() << "wait audio";
		usleep( 1000 );
	}

	f->setAudioFrame( outProfile.getAudioChannels(), outProfile.getAudioSampleRate(), Profile::bytesPerChannel( &outProfile ), nSamples, audioFrameList.readPts() );
	audioFrameList.read( f->data(), nSamples );

	return f;
}
