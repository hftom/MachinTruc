#include "engine/profile.h"



Profile::Profile()
{
	videoFrameRate = 25;
	videoFrameDuration = 40000;
	videoWidth = 1280;
	videoHeight = 720;
	videoAspectRatio = 16./9.;
	videoInterlaced = false;
	videoTopFieldFirst = true;
	videoColorSpace = SPC_UNDEF;
	videoColorPrimaries = PRI_UNDEF;
	videoColorFullRange = true;
	videoChromaLocation = LOC_UNDEF;
	videoGammaCurve = GAMMA_UNDEF;
	
	streamStartTime = 0;
	streamDuration = 0;

	audioSampleRate = DEFAULTSAMPLERATE;
	audioChannels = DEFAULTCHANNELS;
	audioFormat = SAMPLE_FMT_S16;
	audioLayout = DEFAULTLAYOUT;
	
	haveAudio = haveVideo = true;
}



int Profile::bytesPerChannel( Profile *prof )
{
	if ( prof->getAudioFormat() == SAMPLE_FMT_32F )
		return 4;
	return 2;
}