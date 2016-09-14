#ifndef AUDIOMIX_H
#define AUDIOMIX_H

#include <math.h>

#include "afx/audiofilter.h"



class AudioMix : public AudioFilter
{
public:
	AudioMix( QString id = "AudioMix", QString name = "AudioMix" ) : AudioFilter( id, name ) {}

	bool process( Frame *first, Buffer *buf1, Frame *second, Buffer *buf2, Buffer *dst, Profile *p ) {
		Q_UNUSED( p );
		Q_UNUSED( second );
		int i, j, samples = first->audioSamples(), channels = first->profile.getAudioChannels();
		float *in1 = (float*)buf1->data();
		float *in2 = (float*)buf2->data();
		float *out = (float*)dst->data();

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
				v = v < -1 ? -1 : v > 1 ? 1 : v;
				vp[ j ] = out[ i * channels + j ] = (float)( v * A + vp[ j ] * B );
			}
		}

		return true;
	}
};

#endif //AUDIOMIX_H
