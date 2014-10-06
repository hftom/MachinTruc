#ifndef AUDIOVOLUME_H
#define AUDIOVOLUME_H

#include "afx/audiofilter.h"



class AudioVolume : public AudioFilter
{
public:
	AudioVolume( QString id, QString name ) : AudioFilter( id, name ) {
		volume = addParameter( "volume", tr("Volume:"), Parameter::PDOUBLE, 1.0, 0.0, 2.0, true );
	}

	bool process( Frame *src, Profile *p ) {
		Q_UNUSED( p );
		int samples = src->audioSamples(), channels = src->profile.getAudioChannels();
		int16_t *in = (int16_t*)src->data();
		double vol = getParamValue( volume, src->pts() ).toDouble();
		double d = (double)samples * MICROSECOND / (double)src->profile.getAudioSampleRate();
		double vol2 = getParamValue( volume, src->pts() + d ).toDouble();
		d = (vol2 - vol) / samples;

		for ( int i = 0; i < samples; ++i ) {
			for ( int j = 0; j < channels; ++j )
				in[(i * channels) + j] = (double)in[(i * channels) + j] * vol;
			vol += d;
		}
		return true;
	}

private:
	Parameter *volume;
};

#endif //AUDIOVOLUME_H
