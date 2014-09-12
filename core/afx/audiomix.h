#ifndef AUDIOMIX_H
#define AUDIOMIX_H

#include <math.h>

#include "afx/audiofilter.h"



class AudioMix : public AudioFilter
{
public:
	AudioMix( QString id = "AudioMix", QString name = "AudioMix" ) : AudioFilter( id, name ) {}

	bool process( Frame *src, Frame *dst, Profile *p ) {
		Q_UNUSED( p );
		int i, j, samples = src->audioSamples(), channels = src->profile.getAudioChannels();
		int16_t *in = (int16_t*)src->data();
		int16_t *out = (int16_t*)dst->data();

		// MLt combine_audio
		double vp[6];
		for ( j = 0; j < channels; j++ )
			vp[j] = (double)out[j];

		double Fc = 0.5;
		double B = exp( -2.0 * M_PI * Fc );
		double A = 1.0 - B;
		double v;

		for ( i = 0; i < samples; i++ ) {
			for ( j = 0; j < channels; j++ ) {
				v = (double)( 1.0 * out[ i * channels + j ] + in[ i * channels + j ] );
				v = v < -32767 ? -32767 : v > 32768 ? 32768 : v;
				vp[ j ] = out[ i * channels + j ] = (int16_t)( v * A + vp[ j ] * B );
			}
		}

		return true;
	}
};

#endif //AUDIOMIX_H
