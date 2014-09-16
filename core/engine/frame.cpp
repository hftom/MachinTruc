// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include <stdio.h>

#include "frame.h"



Frame::Frame( MQueue<Frame*> *origin, bool makeSample )
	: mmi( 0 ),
	pType( Frame::NONE ),
	fb( NULL ),
	pb( NULL ),
	glfence( NULL ),
	pPTS( 0 ),
	buffer( NULL ),
	originQueue( origin )
{
	if ( makeSample )
		sample = new ProjectSample();
	else
		sample = NULL;
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
	if ( sample ) {
		sample->clear();
		delete sample;
	}
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

	if ( sample )
		sample->clear();

	if ( buffer ) {
		BufferPool::globalInstance()->releaseBuffer( buffer );
		buffer = NULL;
	}

	mmi = 0;

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



void Frame::setVideoFrame( DataType t, int w, int h, double sar, bool il, bool tff, double p, double d )
{
	pType = t;
	profile.setVideoWidth( w );
	profile.setVideoHeight( h );
	profile.setVideoSAR( sar );
	profile.setVideoInterlaced( il );
	profile.setVideoTopFieldFirst( tff );
	profile.setVideoFrameDuration( d );
	pPTS = p;

	int s = w * h;
	switch ( pType ) {
		case YUV420P : s = s * 3 / 2; break;
		case YUV422P : s = s * 2; break;
		case RGBA : s = s * 4; break;
		case RGB : s = s * 3; break;
		default : s = 0;
	}

	if ( s > 0 && !buffer )
		buffer = BufferPool::globalInstance()->getBuffer( s );
}



void Frame::setVideoFrame( Frame *src )
{
	profile = src->profile;
	pPTS = src->pts();
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



void FrameSample::clear( bool releaseFrame )
{
	videoFilters.clear();
	audioFilters.clear();
	if ( frame && releaseFrame ) {
		frame->release();
		frame = NULL;
	}
	transitionFrame.clear( releaseFrame );
}
