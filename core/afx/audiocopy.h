#ifndef AUDIOCOPY_H
#define AUDIOCOPY_H

#include "afx/audiocomposition.h"



class AudioCopy : public AudioComposition
{
	Q_OBJECT
public:
	AudioCopy() {}
	~AudioCopy() {}

	bool process( Frame *src, Frame *dst ) {
		int bps = src->profile.getAudioChannels() * src->profile.bytesPerChannel( &src->profile );
		dst->setAudioFrame( src->profile.getAudioChannels(), src->profile.getAudioSampleRate(), src->profile.bytesPerChannel( &src->profile ), src->audioSamples(), src->pts() );
		mempcpy( dst->data(), src->data(), src->audioSamples() * bps );
		return true;
	}
};

#endif //AUDIOCOPY_H
