// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include <QtConcurrentRun>

#include "input/input_ff.h"

#define BACKWARDLEN MICROSECOND
#define AUDIODRIFTMAX 100000



InputFF::InputFF() : InputBase(),
	decoder( new FFDecoder() ),
	semaphore( new QSemaphore( 1 ) ),
	running( false ),
	seekAndPlayPTS( 0 ),
	seekAndPlay( false ),
	forwardAudioSamples( 0 ),
	forwardStartPts( 0 ),
	backwardAudioSamples( 0 ),
	samplesInBackwardAudioFrames(0),
	playBackward( false ),
	backwardPts( 0 ),
	backwardStartPts( 0 ),
	backwardEof( false ),
	eofVideo( false ),
	eofAudio( false )
{
	inputType = FFMPEG;
}



InputFF::~InputFF()
{
	play( false );
	wait();
	delete decoder;
	flush();
}



void InputFF::flush()
{
	Frame *f;
	while ( (f = reorderedVideoFrames.dequeue()) )
		delete f;
	while ( !backwardVideoFrames.isEmpty() )
		delete backwardVideoFrames.takeFirst();
	while ( !backwardAudioFrames.isEmpty() )
		delete backwardAudioFrames.takeFirst();

	lastFrame.set( NULL );
	audioFrameList.reset( outProfile );
	videoResampler.reset( outProfile.getVideoFrameDuration() );
	eofVideo = eofAudio = false;
	backwardEof = false;
	forwardAudioSamples = 0;
	backwardAudioSamples = 0;
	samplesInBackwardAudioFrames = 0;
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
	--mmi;

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
			samplesInBackwardAudioFrames += af->available;
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
		Frame *f = new Frame();
		AudioFrame *af = new AudioFrame( audioFrameList.getBytesPerSample(), outProfile.getAudioSampleRate() );
		bool ok = decoder->seekTo( p, f, af );
		if ( af->buffer ) {
			audioFrameList.append( af );
			forwardAudioSamples += af->available;
			forwardStartPts = p;
		}
		else {
			delete af;
			eofAudio = true;
		}
		if ( ok && f->getBuffer() ) {
			lastFrame.set( f );
			videoResampler.reset( outProfile.getVideoFrameDuration() );
			videoResampler.outputPts = f->pts();
			resample( f );
			return f->pts();
		}
		else {
			delete f;
			eofVideo = true;
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



void threadOpenSeekPlay( InputFF *that, QString fn, double p, bool backward )
{
	that->osp( fn, p, backward );
}

void InputFF::osp( QString fn, double p, bool backward )
{
	play( false );
	seekAndPlayPTS = p;
	seekAndPlayPath = fn;
	seekAndPlay = true;
	playBackward = backward;
	start();
}

void InputFF::openSeekPlay( QString fn, double p, bool backward )
{
	// We want this function to return as soon as possible,
	// but in some cases it may block too long in play(), particularly
	// if we are seeking in backward mode.
	// Thus, we delegate the work to InputFF::osp that we
	// call from a separate thread.
	semaphore->acquire();
	QtConcurrent::run( threadOpenSeekPlay, this, fn, p, backward );
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
	double oasr = outProfile.getAudioSampleRate();

	while ( running ) {
		doWait = 1;

		if ( decoder->haveVideo && !eofVideo ) {
			if ( reorderedVideoFrames.count() < 3 ) {
				f = new Frame();
				// resample if necessary
				if ( videoResampler.repeat && lastFrame.valid() ) {
					// duplicate previous frame
					lastFrame.get( f );
					videoResampler.duplicate( f );
					f->mmi = mmi;
					f->mmiProvider = mmiProvider;
					reorderedVideoFrames.enqueue( f );
				}
				else {
					mmiIncrement();
					f->mmi = mmi;
					f->mmiProvider = mmiProvider;
					if ( decoder->decodeVideo( f ) ) {
						lastFrame.set( f );
						resample ( f );
					}
					else
						delete f;
				}

				if ( (decoder->endOfFile & FFDecoder::EofVideo) && !videoResampler.repeat )
					eofVideo = true;
				doWait = 0;
			}
		}

		if ( decoder->haveAudio && !eofAudio ) {
			if ( audioFrameList.writable() ) {
				AudioFrame *af = new AudioFrame( audioFrameList.getBytesPerSample(), oasr );
				decoder->decodeAudio( af );
				if ( af->buffer ) {
					double expectedPts = forwardStartPts + ((double)forwardAudioSamples * MICROSECOND / oasr);
					double dpts = af->bufPts - expectedPts;
					if ( dpts > AUDIODRIFTMAX  ) {
						qDebug() << "Inserting silence to compensate audio pts drift : expected" << expectedPts << "got" << af->bufPts;
						AudioFrame *silence = new AudioFrame( audioFrameList.getBytesPerSample(), oasr );
						int ns = qMin( dpts * oasr / MICROSECOND, oasr );
						uint8_t* dst = silence->write( 0, ns );
						memset( dst, 0, ns * silence->bytesPerSample );
						silence->writeDone( expectedPts, ns );
						audioFrameList.append( silence );
						forwardAudioSamples += silence->available;
						audioFrameList.append( af );
						forwardAudioSamples += af->available;
					}
					else if ( dpts < -AUDIODRIFTMAX ) {
						qDebug() << "Dropping samples to compensate audio pts drift : expected" << expectedPts << "got" << af->bufPts;
						int ns = qMin( -dpts * oasr / MICROSECOND, (double)af->available );
						af->available -= ns;
						audioFrameList.append( af );
						forwardAudioSamples += af->available;
					}
					else {
						audioFrameList.append( af );
						forwardAudioSamples += af->available;
					}
				}
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
	int minSamples = (outProfile.getAudioSampleRate() / outProfile.getVideoFrameRate()) * NUMINPUTFRAMES;
	double frameRate = inProfile.getVideoFrameRate() * (1.0 + (decoder->doYadif > FFDecoder::Yadif1X));

	while ( running ) {
		doWait = 1;

		if ( !endVideoSequence ) {
			bool yes = backwardVideoFrames.count() + reorderedVideoFrames.count() < (frameRate * BACKWARDLEN / MICROSECOND) + 3;
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
				if ( backwardVideoFrames.count() > frameRate * 2 ) // broken stream ?
					endVideoSequence = true;
				doWait = 0;
			}
		}

		if ( !endAudioSequence ) {
			bool yes = !audioFrameList.readable( minSamples * 3 );
			yes |= decoder->haveVideo && reorderedVideoFrames.count() < 3;
			if ( yes ) {
				AudioFrame *af = new AudioFrame( audioFrameList.getBytesPerSample(), outProfile.getAudioSampleRate() );
				decoder->decodeAudio( af );
				if ( samplesInBackwardAudioFrames > outProfile.getAudioSampleRate() * 2 ) { // broken stream ?
					delete af;
					endAudioSequence = true;
				}
				else if ( af->available ) {
					double dpts = (double)(backwardAudioSamples + af->available) * MICROSECOND / (double)outProfile.getAudioSampleRate();
					if ( backwardAudioFrames.count() && backwardAudioFrames.first()->bufPts + dpts >= backwardStartPts ) {
						af->available -= ((backwardAudioFrames.first()->bufPts + dpts - backwardStartPts) * (double)outProfile.getAudioSampleRate() / MICROSECOND) - 0.5;
						if ( af->available <= 0 )
							delete af;
						else {
							backwardAudioSamples += af->available;
							samplesInBackwardAudioFrames += af->available;
							backwardAudioFrames.append( af );
						}
						endAudioSequence = true;
					}
					else {
						backwardAudioSamples += af->available;
						samplesInBackwardAudioFrames += af->available;
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
			double bpts = backwardPts;
			if ( backwardVideoFrames.count() ) {
				bpts = backwardVideoFrames.first()->pts();
				if ( backwardVideoFrames.first()->pts() <= inProfile.getStreamStartTime() )
					backwardEof = true;
			}
			else if ( backwardAudioFrames.count() ) {
				bpts = backwardAudioFrames.first()->bufPts;
				if ( backwardAudioFrames.first()->bufPts <= inProfile.getStreamStartTime() )
					backwardEof = true;
			}
			else
				backwardEof = true;

			while ( !backwardVideoFrames.isEmpty() ) {
				// resample if necessary
				if ( videoResampler.repeat && lastFrame.valid() ) {
					// duplicate previous frame
					Frame *f = new Frame( NULL );
					lastFrame.get( f, true );
					videoResampler.duplicate( f, true );
					f->mmi = mmi;
					f->mmiProvider = mmiProvider;
					reorderedVideoFrames.enqueue( f );
				}
				else {
					Frame *f = backwardVideoFrames.takeLast();
					mmiIncrement();
					f->mmi = mmi;
					f->mmiProvider = mmiProvider;
					lastFrame.set( f );
					resampleBackward( f );
				}
				bpts = videoResampler.outputPts + videoResampler.outputDuration;
			}

			// we don't want to freeze for ever in broken streams
			if ( bpts >= backwardPts )
				backwardPts -= BACKWARDLEN / 2.0;
			else
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
			samplesInBackwardAudioFrames = 0;

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
					samplesInBackwardAudioFrames += af->available;
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
	double duration = (decoder->doYadif > FFDecoder::Yadif1X) ? f->profile.getVideoFrameDuration() / 2.0 : f->profile.getVideoFrameDuration();
	double delta = ( f->pts() + duration ) - ( videoResampler.outputPts + videoResampler.outputDuration );
	/*if ( outProfile.getVideoFrameRate() == inProfile.getVideoFrameRate() ) { // no resampling
		videoFrames.enqueue( f );
	}
	else*/ if ( delta >= videoResampler.outputDuration ) {
		// this frame will be duplicated (delta / videoResampler.outputDuration) times
		printf("duplicate, delta=%f, f->pts=%f, outputPts=%f\n", delta, f->pts(), videoResampler.outputPts);
		// add some to delta to prevent subduplicate
		videoResampler.setRepeat( f->pts(), (delta + 1) / videoResampler.outputDuration );
		reorderedVideoFrames.enqueue( f );
		videoResampler.outputPts += videoResampler.outputDuration;
	}
	else if ( delta <= -videoResampler.outputDuration ) {
		// skip
		printf("skip frame delta=%f, f->pts=%f, outputPts=%f\n", delta, f->pts(), videoResampler.outputPts);
		delete f;
	}
	else {
		reorderedVideoFrames.enqueue( f );
		videoResampler.outputPts += videoResampler.outputDuration;
	}
}



void InputFF::resampleBackward( Frame *f )
{
	double duration = (decoder->doYadif > FFDecoder::Yadif1X) ? f->profile.getVideoFrameDuration() / 2.0 : f->profile.getVideoFrameDuration();
	double delta = ( videoResampler.outputPts - videoResampler.outputDuration ) - ( f->pts() - duration );
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

	while ( reorderedVideoFrames.queueEmpty() ) {
		if ( eofVideo ) {
			if ( lastFrame.valid() ) {
				Frame *f = new Frame();
				lastFrame.get( f );
				//if ( sourceName.endsWith(".MOD") )
					//f->profile.setVideoSAR( 1024.0/704.0 );
				return f;
			}
			return NULL;
		}
		usleep( 1000 );
	}

	Frame *f = reorderedVideoFrames.dequeue();
	//if ( sourceName.endsWith(".MOD") )
		//f->profile.setVideoSAR( 1024.0/704.0 );
	return f;
}



Frame* InputFF::getAudioFrame( int nSamples )
{
	SemaphoreLocker sem( semaphore );

	if ( !decoder->haveAudio ) {
		return NULL;
	}

	Frame *f = new Frame();

	while ( !audioFrameList.readable( nSamples ) ) {
		if ( eofAudio ) {
			f->setAudioFrame( outProfile.getAudioChannels(), outProfile.getAudioSampleRate(), Profile::bytesPerChannel( &outProfile ), nSamples, audioFrameList.readPts() );
			int n = audioFrameList.read( f->data() );
			if ( !n ) {
				delete f;
				return NULL;
			}
			// complete with silence
			memset( f->data() + ( n * audioFrameList.getBytesPerSample() ), 0, (nSamples - n) * audioFrameList.getBytesPerSample() );
			f->audioReversed = speed < 0 ? !playBackward : playBackward;
			return f;
		}
		//qDebug() << "wait audio";
		usleep( 1000 );
	}

	f->setAudioFrame( outProfile.getAudioChannels(), outProfile.getAudioSampleRate(), Profile::bytesPerChannel( &outProfile ), nSamples, audioFrameList.readPts() );
	audioFrameList.read( f->data(), nSamples );
	f->audioReversed = speed < 0 ? !playBackward : playBackward;

	return f;
}
