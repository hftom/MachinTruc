#include "engine/profile.h"



Profile::Profile()
	: videoFrameRate( 25 ),
	videoFrameDuration( 40000 ),
	videoWidth( 1280 ),
	videoHeight( 720 ),
	videoSAR( 1.0 ),
	videoInterlaced( false ),
	videoTopFieldFirst( true ),
	videoColorSpace( SPC_UNDEF ),
	videoColorPrimaries( PRI_UNDEF ),
	videoChromaLocation( LOC_UNDEF ),
	videoGammaCurve( GAMMA_UNDEF ),
	videoColorFullRange( true ),
	haveVideo( true ),
	haveAudio( true ),
	audioSampleRate( DEFAULTSAMPLERATE ),
	audioChannels( DEFAULTCHANNELS ),
	audioFormat( SAMPLE_FMT_S16 ),
	audioLayout( DEFAULTLAYOUT ),
	streamStartTime( 0 ),
	streamDuration( 0 )
{
}



int Profile::bytesPerChannel( Profile *prof )
{
	if ( prof->getAudioFormat() == SAMPLE_FMT_32F )
		return 4;
	return 2;
}