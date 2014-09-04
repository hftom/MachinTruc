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



QString Profile::colorPrimariesName()
{
	switch ( videoColorPrimaries ) {
		case PRI_601_625 : return QObject::tr( "BT.470BG" );
		case PRI_601_525 : return QObject::tr( "SMPTE-240M" );
		case PRI_SRGB : return QObject::tr( "sRGB" );
		case PRI_709 :
		default : return QObject::tr( "BT.709" );
	}
}



QString Profile::gammaCurveName()
{
	switch ( videoGammaCurve ) {
		case GAMMA_601 : return QObject::tr( "SMPTE-170M" );
		case GAMMA_SRGB : return QObject::tr( "sRGB" );
		case GAMMA_709 : 
		default : return QObject::tr( "BT.709" );
	}
}



QString Profile::colorSpaceName()
{
	switch ( videoColorSpace ) {
		case SPC_601_625 : return QObject::tr( "BT.470BG" );
		case SPC_601_525 : return QObject::tr( "SMPTE-170M" );
		case SPC_SRGB : return QObject::tr( "sRGB" );
		case SPC_709 : 
		default : return QObject::tr( "BT.709" );
	}
}



int Profile::bytesPerChannel( Profile *prof )
{
	if ( prof->getAudioFormat() == SAMPLE_FMT_32F )
		return 4;
	return 2;
}
