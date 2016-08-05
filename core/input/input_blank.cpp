// kate: tab-indent on; indent-width 4; mixedindent off; indent-mode cstyle; remove-trailing-space on;

#include "input/input_blank.h"

#define DEFAULTLENGTH 5.0 * MICROSECOND



InputBlank::InputBlank() : InputBase(),
	currentVideoPTS( 0 )
{
	inputType = GLSL;
}



bool InputBlank::probe( QString fn, Profile *prof )
{
	if ( !open( fn ) )
		return false;

	prof->setVideoWidth( 1280 );
	prof->setVideoHeight( 720 );
	prof->setVideoSAR( 1.0 );
	prof->setVideoFrameRate( 25.0 );
	prof->setVideoFrameDuration( MICROSECOND / 25.0 );
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
	return fn.startsWith("Blank");
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

	double dur = MICROSECOND / 25.0;
	qint64 i = p / dur;
	currentVideoPTS = i * dur;

	return currentVideoPTS;
}



void InputBlank::upload( Frame *f )
{
	f->mmi = mmi;
	f->mmiProvider = mmiProvider;
	f->setVideoFrame( Frame::GLSL, outProfile.getVideoWidth(), outProfile.getVideoHeight(), outProfile.getVideoSAR(), false, false, currentVideoPTS, outProfile.getVideoFrameDuration() );
	mmiDuplicate();
	currentVideoPTS += outProfile.getVideoFrameDuration();
}



Frame* InputBlank::getVideoFrame()
{
	Frame *f = new Frame();
	upload( f );
	return f;
}
