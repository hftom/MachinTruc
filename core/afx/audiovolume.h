#ifndef AUDIOVOLUME_H
#define AUDIOVOLUME_H

#include "afx/audiofilter.h"



class AudioVolume : public AudioFilter
{
	Q_OBJECT
public:
	AudioVolume( QString id, QString name ) : AudioFilter( id, name ) {
		volume = 1.0;
		addParameter( tr("Volume:"), PFLOAT, 0.0, 2.0, true, &volume );
	}
	~AudioVolume() {}

	bool process( Frame *src ) {
		int i, j, samples = src->audioSamples(), channels = src->profile.getAudioChannels();
		int16_t *in = (int16_t*)src->data();

		for ( i = 0; i < samples; ++i ) {
			for ( j = 0; j < channels; ++j )
				in[(i * channels) + j] = (float)in[(i * channels) + j] * volume;
		}
		return true;
	}

private:
	float volume;
};

#endif //AUDIOVOLUME_H
