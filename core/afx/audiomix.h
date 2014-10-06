#ifndef AUDIOMIX_H
#define AUDIOMIX_H

#include <math.h>

#include "afx/audiofilter.h"



class AudioMix : public AudioFilter
{
public:
	AudioMix( QString id = "AudioMix", QString name = "AudioMix" ) : AudioFilter( id, name ) {}

	bool process( Frame *fisrt, Frame *second, Profile *p ) {
		Q_UNUSED( p );
		int i, j, samples = fisrt->audioSamples(), channels = fisrt->profile.getAudioChannels();
		int16_t *in1 = (int16_t*)fisrt->data();
		int16_t *in2 = (int16_t*)second->data();

		// MLt combine_audio
		double vp[6];
		for ( j = 0; j < channels; j++ )
			vp[j] = (double)in2[j];

		double Fc = 0.5;
		double B = exp( -2.0 * M_PI * Fc );
		double A = 1.0 - B;
		double v;

		for ( i = 0; i < samples; i++ ) {
			for ( j = 0; j < channels; j++ ) {
				v = (double)( 1.0 * in2[ i * channels + j ] + in1[ i * channels + j ] );
				v = v < -32767 ? -32767 : v > 32767 ? 32767 : v;
				vp[ j ] = in2[ i * channels + j ] = (int16_t)( v * A + vp[ j ] * B );
			}
		}

		return true;
	}
};

#endif //AUDIOMIX_H
