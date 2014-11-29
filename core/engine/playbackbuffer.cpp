#include "playbackbuffer.h"



PlaybackBuffer::PlaybackBuffer()
	: maxFrames( 25 ),
	backward( false ),
	skipPts( -1 )
{
}



PlaybackBuffer::~PlaybackBuffer()
{
}

	
	
void PlaybackBuffer::reset( int framerate, double skip )
{
	QMutexLocker ml( &mutex );
	while ( !videoSamples.isEmpty() )
		delete videoSamples.takeFirst();
	while ( !audioSamples.isEmpty() )
		delete audioSamples.takeFirst();
	maxFrames = framerate;
	skipPts = skip;
	backward = false;
}



int PlaybackBuffer::getBuffer( double pts, bool back )
{
	QMutexLocker ml( &mutex );
	backward = back;
	
	if ( backward ) {
		for ( int i = videoSamples.count() - 1; i >= 0; --i ) {
			if ( videoSamples.at( i )->pts == pts ) {
				int nvideo = i + 1;
				int naudio = 0;
				int j;
				for ( j = audioSamples.count() - 1; j >= 0; --j ) {
					if ( audioSamples.at( j )->pts == pts ) {
						naudio = j + 1;
						break;
					}
				}
				if ( naudio < nvideo )
					nvideo = naudio;
				int loop = i + 1 - nvideo;
				for ( int k = 0; k < loop; ++k )
					delete videoSamples.takeFirst();
				loop = j + 1 - nvideo;
				for ( int k = 0; k < loop; ++k )
					delete audioSamples.takeFirst();
				
				return nvideo;
			}	
		}
	}
	else {
		for ( int i = 0; i < videoSamples.count(); ++i ) {
			if ( videoSamples.at( i )->pts == pts ) {
				int nvideo = videoSamples.count() - i;
				int naudio = 0;
				int j;
				for ( j = 0; j < audioSamples.count(); ++j ) {
					if ( audioSamples.at( j )->pts == pts ) {
						naudio = audioSamples.count() - j;
						break;
					}
				}
				if ( naudio < nvideo )
					nvideo = naudio;
				int loop = videoSamples.count() - i - nvideo;
				for ( int k = 0; k < loop; ++k )
					delete videoSamples.takeLast();
				loop = audioSamples.count() - j - nvideo;
				for ( int k = 0; k < loop; ++k )
					delete audioSamples.takeLast();
				
				return nvideo;
			}	
		}
	}
	return 0;
}



ProjectSample* PlaybackBuffer::getVideoSample( double pts )
{
	QMutexLocker ml( &mutex );

	for ( int i = 0; i < videoSamples.count(); ++i ) {
		if ( videoSamples.at( i )->pts == pts ) {
			BufferedSample *bs = videoSamples.takeAt( i );
			ProjectSample *ps = bs->sample;
			bs->sample = NULL;
			delete bs;
			return ps;
		}
	}
	
	return NULL;
}
	


ProjectSample* PlaybackBuffer::getAudioSample( double pts )
{
	QMutexLocker ml( &mutex );

	for ( int i = 0; i < audioSamples.count(); ++i ) {
		if ( audioSamples.at( i )->pts == pts ) {
			BufferedSample *bs = audioSamples.takeAt( i );
			ProjectSample *ps = bs->sample;
			bs->sample = NULL;
			delete bs;
			checkAudioReversed( ps );
			return ps;
		}
	}
	
	return NULL;
}



void PlaybackBuffer::releasedVideoFrame( Frame *f )
{
	QMutexLocker ml( &mutex );
	bool add = true;
	
	if ( skipPts != -1 && f->pts() == skipPts ) {
		f->release();
		skipPts = -1;
		return;
	}
	
	if ( backward ) {
		for ( int i = 0; i < videoSamples.count(); ++i ) {
			if ( f->pts() < videoSamples.at( i )->pts ) {
				videoSamples.insert( i, new BufferedSample( f ) );
				add = false;
				break;
			}
		}
		if ( add )
			videoSamples.append( new BufferedSample( f ) );
		if ( videoSamples.count() > maxFrames )
			delete videoSamples.takeLast();
	}
	else {
		for ( int i = videoSamples.count() - 1; i >= 0; --i ) {
			if ( f->pts() > videoSamples.at( i )->pts ) {
				videoSamples.insert( i + 1, new BufferedSample( f ) );
				add = false;
				break;
			}
		}
		if ( add )
			videoSamples.prepend( new BufferedSample( f ) );
		if ( videoSamples.count() > maxFrames )
			delete videoSamples.takeFirst();
	}
}

	
	
void PlaybackBuffer::releasedAudioFrame( Frame *f )
{
	QMutexLocker ml( &mutex );
	bool add = true;
	
	if ( backward ) {
		for ( int i = 0; i < audioSamples.count(); ++i ) {
			if ( f->pts() < audioSamples.at( i )->pts ) {
				audioSamples.insert( i, new BufferedSample( f ) );
				add = false;
				break;
			}
		}
		if ( add )
			audioSamples.append( new BufferedSample( f ) );
		if ( audioSamples.count() > maxFrames * 3 / 2 )
			delete audioSamples.takeLast();
	}
	else {
		for ( int i = audioSamples.count() - 1; i >= 0; --i ) {
			if ( f->pts() > audioSamples.at( i )->pts ) {
				audioSamples.insert( i + 1, new BufferedSample( f ) );
				add = false;
				break;
			}
		}
		if ( add )
			audioSamples.prepend( new BufferedSample( f ) );
		if ( audioSamples.count() > maxFrames * 3 / 2 )
			delete audioSamples.takeFirst();
	}
}



void PlaybackBuffer::checkAudioReversed( ProjectSample *sample )
{
	for ( int i = 0; i < sample->frames.count(); ++i ) {
		FrameSample *fs = sample->frames[i];
		if ( fs->frame && fs->frame->audioReversed != backward )
			reverseAudio( fs->frame );
		if ( fs->transitionFrame.frame && fs->transitionFrame.frame->audioReversed != backward )
			reverseAudio( fs->transitionFrame.frame );
	}
}



void PlaybackBuffer::reverseAudio( Frame *f )
{
	int bps = Profile::bytesPerChannel( &f->profile ) * f->profile.getAudioChannels();
	int size = f->audioSamples() * bps;
	Buffer *buffer = BufferPool::globalInstance()->getBuffer( size );
	uint8_t *src = f->data() + size - bps;
	uint8_t *dst = buffer->data();
	while ( size ) {
		memcpy( dst, src, bps );
		src -= bps;
		dst += bps;
		size -= bps;
	}
	f->setSharedBuffer( buffer );
	BufferPool::globalInstance()->releaseBuffer( buffer );
	f->audioReversed = !f->audioReversed;
}
