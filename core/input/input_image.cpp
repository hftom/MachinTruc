// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include <unistd.h>
#include <libexif/exif-data.h>

#include <QFile>

#include "input/input_image.h"

#define DEFAULTLENGTH 5.0 * MICROSECOND



InputImage::InputImage() : InputBase(),
	orientation(0),
	fps( 25 ),
	currentVideoPTS( 0 ),
	buffer( NULL ),
	width( 0 ),
	height( 0 ),
	rgba( false )
{
	inputType = IMAGE;
	semaphore = new QSemaphore( 1 );
}



InputImage::~InputImage()
{
	if ( buffer )
		BufferPool::globalInstance()->releaseBuffer( buffer );

	delete semaphore;
}



bool InputImage::probe( QString fn, Profile *prof )
{
	prof->setHasVideo( false );
	prof->setHasAudio( false );

	if ( !open( fn ) )
		return false;

	prof->setVideoWidth( width );
	prof->setVideoHeight( height );
	prof->setVideoSAR( 1.0 );
	prof->setVideoFrameRate( fps );
	prof->setVideoFrameDuration( MICROSECOND / fps );
	prof->setStreamStartTime( 0 );
	prof->setStreamDuration( DEFAULTLENGTH );
	prof->setVideoColorSpace( Profile::SPC_SRGB );
	prof->setVideoColorPrimaries( Profile::PRI_SRGB );
	prof->setVideoGammaCurve( Profile::GAMMA_SRGB );
	prof->setVideoCodecName( "image" );
	prof->setHasVideo( true );
	return true;
}



void InputImage::getOrientation()
{
	orientation = 0;
	ExifData *ed = exif_data_new_from_file(sourceName.toLocal8Bit().data());
	if (ed) {
		ExifEntry *ee = exif_data_get_entry(ed, EXIF_TAG_ORIENTATION);
		if (ee && ee->size == 2) {
			int orient = exif_get_short(ee->data, EXIF_BYTE_ORDER_INTEL);
			if (orient < 1 || orient > 8) {
				orient = exif_get_short(ee->data, EXIF_BYTE_ORDER_MOTOROLA);
			}
			if (orient > 1 && orient < 9) {
				orientation = orient;
			}
		}
	}
}



void InputImage::run()
{
	open( sourceName );
	semaphore->release();
}



bool InputImage::open( QString fn )
{
	sourceName = fn;

	mmiSeek();

	if ( buffer ) {
		BufferPool::globalInstance()->releaseBuffer( buffer );
		buffer = NULL;
	}

	QImage image;
	if ( !image.load( sourceName ) )
		return false;

	if ( image.depth() != 24 && image.depth() != 32 )
		return false;

	if ( image.width() > 1920 || image.height() > 1080 )
		image = image.scaled( 1920, 1080, Qt::KeepAspectRatio, Qt::SmoothTransformation );

	if ( image.isNull() )
		return false;

	width = image.width();
	height = image.height();
	rgba = image.depth() == 32;
	buffer = BufferPool::globalInstance()->getBuffer( image.byteCount() );
	memcpy( buffer->data(), image.constBits(), image.byteCount() );

	getOrientation();

	return true;
}



void InputImage::openSeekPlay( QString fn, double p, bool backward )
{
	Q_UNUSED(backward);
	if ( fn != sourceName || !buffer ) {
		wait();
		semaphore->acquire();
		sourceName = fn;
		start();
	}
	seekTo( p );
}



double InputImage::seekTo( double p )
{
	mmiSeek();
	
	double dur = MICROSECOND / fps;
	qint64 i = p / dur;
	currentVideoPTS = i * dur;

	return currentVideoPTS;
}



bool InputImage::upload( Frame *f )
{
	semaphore->acquire();

	if ( !buffer ) {
		semaphore->release();
		return false;
	}

	f->setSharedBuffer( buffer );
	f->mmi = mmi;
	f->mmiProvider = mmiProvider;
	f->setVideoFrame( rgba ? Frame::RGBA : Frame::RGB, width, height, 1.0, false, false, currentVideoPTS, outProfile.getVideoFrameDuration(), orientation );

	mmiDuplicate();
	currentVideoPTS += outProfile.getVideoFrameDuration();

	semaphore->release();
	return true;
}



Frame* InputImage::getVideoFrame()
{
	Frame *f = new Frame();

	if ( !upload( f ) ) {
		delete f;
		return NULL;
	}
	return f;
}
