#ifndef AUDIOCOPY_H
#define AUDIOCOPY_H

#include "afx/audiofilter.h"



class AudioCopy : public AudioFilter
{
	Q_OBJECT
public:
	AudioCopy( QString id = "AudioCopy", QString name = "AudioCopy" ) : AudioFilter( id, name ) {}

	bool process( Frame *src, Frame *dst, Profile *p ) {
		Q_UNUSED( p );
		int bps = src->profile.getAudioChannels() * src->profile.bytesPerChannel( &src->profile );
		dst->setAudioFrame( src->profile.getAudioChannels(), src->profile.getAudioSampleRate(), src->profile.bytesPerChannel( &src->profile ), src->audioSamples(), src->pts() );
		mempcpy( dst->data(), src->data(), src->audioSamples() * bps );
		return true;
	}
};

#endif //AUDIOCOPY_H
