// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include "input/input_blank.h"

#define DEFAULTLENGTH 5.0 * MICROSECOND



InputBlank::InputBlank() : InputBase(),
	fps( 25 ),
	currentVideoPTS( 0 ),
	width( 0 ),
	height( 0 )
{
	inputType = GLSL;
}



bool InputBlank::probe( QString fn, Profile *prof )
{
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
	prof->setVideoCodecName( "glsl" );
	prof->setHasVideo( true );
	prof->setHasAudio( false );
	return true;
}



bool InputBlank::open( QString fn )
{
	mmiSeek();

	QStringList sl = fn.split(" ");
	if ( sl.count() != 3 || sl.first() != "Blank" )
		return false;

	width = qMax( 1, sl[1].toInt() );
	height = qMax( 1, sl[2].toInt() );
	return true;
}



void InputBlank::openSeekPlay( QString fn, double p, bool backward )
{
	Q_UNUSED( backward );
	open( fn );
	seekTo( p );
}



double InputBlank::seekTo( double p )
{
	mmiSeek();

	double dur = MICROSECOND / fps;
	qint64 i = p / dur;
	currentVideoPTS = i * dur;

	return currentVideoPTS;
}



void InputBlank::upload( Frame *f )
{
	f->mmi = mmi;
	f->mmiProvider = mmiProvider;
	f->setVideoFrame( Frame::GLSL, width, height, 1.0, false, false, currentVideoPTS, outProfile.getVideoFrameDuration() );
	mmiDuplicate();
	currentVideoPTS += outProfile.getVideoFrameDuration();
}



Frame* InputBlank::getVideoFrame()
{
	Frame *f = new Frame();
	upload( f );
	return f;
}
