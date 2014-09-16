#ifndef AUDIOCOPY_H
#define AUDIOCOPY_H

#include "afx/audiofilter.h"



class AudioCopy : public AudioFilter
{
	Q_OBJECT
public:
	AudioCopy( QString id = "AudioCopy", QString name = "AudioCopy" ) : AudioFilter( id, name ) {}

	bool process( Frame *first, Frame *second, Profile *p ) {
		Q_UNUSED( p );
		int bps = first->profile.getAudioChannels() * first->profile.bytesPerChannel( &first->profile );
		second->setAudioFrame( first->profile.getAudioChannels(), first->profile.getAudioSampleRate(), first->profile.bytesPerChannel( &first->profile ), first->audioSamples(), first->pts() );
		mempcpy( second->data(), first->data(), first->audioSamples() * bps );
		return true;
	}
};

#endif //AUDIOCOPY_H
