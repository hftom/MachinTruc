#ifndef AUDIOCOPY_H
#define AUDIOCOPY_H

#include "afx/audiofilter.h"



class AudioCopy : public AudioFilter
{
	Q_OBJECT
public:
	AudioCopy( QString id = "AudioCopy", QString name = "AudioCopy" ) : AudioFilter( id, name ) {}

	bool process( Frame *first, Buffer *scr, Buffer *dst, Profile *p ) {
		Q_UNUSED( p );
		int bps = first->profile.getAudioChannels() * Profile::bytesPerChannel( &first->profile );
		mempcpy( dst->data(), scr->data(), first->audioSamples() * bps );
		return true;
	}
};

#endif //AUDIOCOPY_H
