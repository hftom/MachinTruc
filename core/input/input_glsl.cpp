#include "input/input_glsl.h"

#define DEFAULTLENGTH 5.0 * MICROSECOND
#define DEFAULTFPS 30.0



InputGLSL::InputGLSL() : InputBase(),
	currentVideoPTS( 0 )
{
	inputType = GLSL;
}



bool InputGLSL::probe( QString fn, Profile *prof )
{
	if ( !open( fn ) )
		return false;

	prof->setVideoWidth( 1280 );
	prof->setVideoHeight( 720 );
	prof->setVideoSAR( 1.0 );
	prof->setVideoFrameRate( DEFAULTFPS );
	prof->setVideoFrameDuration( MICROSECOND / DEFAULTFPS );
	prof->setStreamStartTime( 0 );
	prof->setStreamDuration( DEFAULTLENGTH );
	prof->setVideoColorSpace( Profile::SPC_SRGB );
	prof->setVideoColorPrimaries( Profile::PRI_SRGB );
	prof->setVideoGammaCurve( Profile::GAMMA_SRGB );
	prof->setVideoCodecName( fn.replace("GLSL_", ""));
	prof->setHasVideo( true );
	prof->setHasAudio( false );
	return true;
}



bool InputGLSL::open( QString fn )
{
	mmiSeek();
	sourceName = fn;
	return fn.startsWith("GLSL_");
}



void InputGLSL::openSeekPlay( QString fn, double p, bool backward )
{
	Q_UNUSED( backward );
	open( fn );
	seekTo( p );
}



double InputGLSL::seekTo( double p )
{
	mmiSeek();

	double dur = MICROSECOND / DEFAULTFPS;
	qint64 i = p / dur;
	currentVideoPTS = i * dur;

	return currentVideoPTS;
}



void InputGLSL::upload( Frame *f )
{
	f->mmi = mmi;
	f->mmiProvider = mmiProvider;
	f->setVideoFrame( Frame::GLSL, outProfile.getVideoWidth(), outProfile.getVideoHeight(), outProfile.getVideoSAR(), false, false, currentVideoPTS, outProfile.getVideoFrameDuration() );
	f->profile.setVideoCodecName(sourceName.replace("GLSL_", ""));
	mmiIncrement();
	currentVideoPTS += outProfile.getVideoFrameDuration();
}



Frame* InputGLSL::getVideoFrame()
{
	Frame *f = new Frame();
	upload( f );
	return f;
}
