// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include <stdio.h>

#include "frame.h"



Frame::Frame( MQueue<Frame*> *origin )
	: audioReversed( false ),
	mmi( 0 ),
	sample( NULL ),
	isDuplicate( false ),
	pType( Frame::NONE ),
	fb( NULL ),
	pb( NULL ),
	glfence( NULL ),
	pPTS( 0 ),
	pOrientation( 0 ),
	buffer( NULL ),
	originQueue( origin )
{
}



Frame::~Frame()
{
	if ( buffer )
		BufferPool::globalInstance()->releaseBuffer( buffer );
	if ( fb )
		fb->setFree( true );
	if ( pb )
		pb->setFree( true );
	if ( glfence )
		glfence->setFree();
	if ( sample )
		delete sample;
}



void Frame::release()
{
	if ( fb ) {
		fb->setFree( true );
		fb = NULL;
	}

	if ( pb ) {
		pb->setFree( true );
		pb = NULL;
	}

	if ( glfence ) {
		glfence->setFree();
		glfence = NULL;
	}

	if ( sample ) {
		delete sample;
		sample = NULL;
	}


	if ( buffer ) {
		BufferPool::globalInstance()->releaseBuffer( buffer );
		buffer = NULL;
	}

	mmi = 0;
	audioReversed = false;
	isDuplicate = false;

	if ( originQueue )
		originQueue->enqueue( this );
}



void Frame::setSharedBuffer( Buffer *b )
{
	if ( buffer )
		BufferPool::globalInstance()->releaseBuffer( buffer );
	buffer = b;
	BufferPool::globalInstance()->useBuffer( buffer );
}



void Frame::setVideoFrame( DataType t, int w, int h, double sar, bool il, bool tff, double p, double d, int rot, int size, int bits )
{
	pType = t;
	bitDepth = bits;
	profile.setVideoWidth( w );
	profile.setVideoHeight( h );
	profile.setVideoSAR( sar );
	profile.setVideoInterlaced( il );
	profile.setVideoTopFieldFirst( tff );
	profile.setVideoFrameDuration( d );
	pPTS = p;
	pOrientation = rot;

	if (!size) {
		size = w * h;
		int bytes = bitDepth > 8 ? 2 : 1;
		switch ( pType ) {
			case YUV420P : size *= 3 / 2 * bytes; break;
			case YUV422P : size *= 2 * bytes; break;
			case YUV444P : size *= 3 * bytes; break;
			case RGBA : size *= 4; break;
			case RGB : size *= 3; break;
			default : size = 0;
		}
	}

	if ( size > 0 && !buffer )
		buffer = BufferPool::globalInstance()->getBuffer( size );
}



void Frame::setVideoFrame( Frame *src )
{
	profile = src->profile;
	pPTS = src->pts();
	pOrientation = src->orientation();
	bitDepth = src->bitDepth;
}



void Frame::setFBO( FBO *f )
{
	pType = GLTEXTURE;
	if ( fb )
		fb->setFree( true );
	fb = f;
}



void Frame::setPBO( PBO *p )
{
	if ( pb )
		pb->setFree( true );
	pb = p;
}



void Frame::setFence( FENCE *f )
{
	if ( glfence )
		glfence->setFree();
	glfence = f;
}



void Frame::setAudioFrame( int c, int r, int bpc, int samples, double p )
{
	profile.setAudioChannels( c );
	profile.setAudioSampleRate( r );
	pAudioSamples = samples;
	pPTS = p;

	int s = c * bpc * samples;
	if ( s > 0 ) {
		if ( buffer )
			BufferPool::globalInstance()->releaseBuffer( buffer );
		buffer = BufferPool::globalInstance()->getBuffer( s );
	}
}
