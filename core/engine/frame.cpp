// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include <stdio.h>

#include "vfx/glfilter.h"
#include "afx/audiofilter.h"
#include "engine/frame.h"



Frame::Frame( MQueue<Frame*> *origin, bool makeSample )
{
	fb = NULL;
	pb = NULL;
	glfence = NULL;
	buffer = NULL;
	bufferSize = 0;
	pType = Frame::NONE;
	pPTS = 0;
	originQueue = origin;
	if ( makeSample )
		sample = new ProjectSample();
	else
		sample = NULL;
}



Frame::~Frame()
{
	if ( buffer )
		free( buffer );
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

	if ( originQueue )
		originQueue->enqueue( this );
}



void Frame::resizeBuffer( int s )
{
	if ( bufferSize < s ) {
		if ( !buffer ) {
			buffer = (uint8_t*)malloc( s );
		}
		else {
			buffer = (uint8_t*)realloc( buffer, s );
		}
		bufferSize = s;
	}
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
	resizeBuffer( s );
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
	resizeBuffer( s );
}



void FrameSample::clear( bool releaseFrame )
{
	while ( !videoFilters.isEmpty() )
		videoFilters.takeFirst()->release();
	while ( !audioFilters.isEmpty() )
		audioFilters.takeFirst()->release();
	composition = NULL;
	if ( frame && releaseFrame ) {
		frame->release();
		frame = NULL;
	}
}
