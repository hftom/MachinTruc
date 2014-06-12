// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include <unistd.h>

#include <QFile>

#include "input/input_image.h"

#define DEFAULTLENGTH 5.0 * MICROSECOND



InputImage::InputImage() : InputBase()
{
	inputType = IMAGE;
	currentVideoPTS = 0;
	fps = 30;

	int i;
	for ( i = 0; i < NUMINPUTFRAMES; ++i )
		freeVideoFrames.enqueue( new Frame( &freeVideoFrames ) );
}



InputImage::~InputImage()
{
	Frame *f;
	while ( (f = freeVideoFrames.dequeue()) )
		delete f;
}



bool InputImage::probe( QString fn, Profile *prof )
{
	prof->setHasVideo( false );
	prof->setHasAudio( false );

	if ( !open( fn ) )
		return false;

	prof->setVideoWidth( image.width() );
	prof->setVideoHeight( image.height() );
	prof->setVideoAspectRatio( (double)image.width() / (double)image.height() );
	prof->setVideoFrameRate( fps );
    prof->setVideoFrameDuration( MICROSECOND / fps );
    prof->setStreamStartTime( 0 );
    prof->setStreamDuration( DEFAULTLENGTH );
    prof->setVideoColorPrimaries( Profile::PRI_709 );
    prof->setVideoGammaCurve( Profile::GAMMA_SRGB );
    prof->setVideoCodecName( "image" );
	prof->setHasVideo( true );
	return true;
}



void InputImage::run()
{
	open( sourceName );
	mutex.unlock();
}



bool InputImage::open( QString fn )
{
	sourceName = fn;

	if ( !image.load( sourceName ) )
		return false;

	if ( image.depth() != 24 && image.depth() != 32 )
		return false;

	if ( image.width() > 1920 || image.height() > 1080 )
		image = image.scaled( 1920, 1080, Qt::KeepAspectRatio, Qt::SmoothTransformation );

	return !image.isNull();
}



void InputImage::openSeekPlay( QString fn, double p )
{
	if ( fn != sourceName ) {
		wait();
		mutex.lock();
		sourceName = fn;
		start();
	}
	seekTo( p );
}



void InputImage::seekFast( float percent )
{
	seekTo( DEFAULTLENGTH * percent / 100.0 );
}



double InputImage::seekTo( double p )
{
	double dur = MICROSECOND / fps;
	qint64 i = p / dur;
	currentVideoPTS = i * dur;

	return currentVideoPTS;
}



bool InputImage::upload( Frame *f )
{
	QMutexLocker m( &mutex );

	if ( image.isNull() || (image.depth()!=24 && image.depth()!=32) )
		return false;
	f->setVideoFrame( (image.depth() == 24) ? Frame::RGB : Frame::RGBA, image.width(), image.height(), (double)image.width() / (double)image.height(), false, false, currentVideoPTS, outProfile.getVideoFrameDuration() );
	memcpy( f->data(), image.constBits(), image.byteCount() );

	currentVideoPTS += outProfile.getVideoFrameDuration();
	return true;
}



Frame* InputImage::getVideoFrame()
{
	Frame *f = NULL;

	while ( freeVideoFrames.queueEmpty() )
		usleep( 500 );

	f = freeVideoFrames.dequeue();
	if ( !upload( f ) ) {
		f->release();
		return NULL;
	}
	return f;
}
