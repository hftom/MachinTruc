#ifndef AUDIOVOLUME_H
#define AUDIOVOLUME_H

#include "afx/audiofilter.h"



class AudioVolume : public AudioFilter
{
	Q_OBJECT
public:
	AudioVolume( QString id, QString name ) : AudioFilter( id, name ) {
		volume = addParameter( "volume", tr("Volume:"), Parameter::PDOUBLE, 1.0, 0.0, 2.0, true );
	}

	bool process( Frame *f, Buffer *src, Buffer *dst, Profile *p ) {
		Q_UNUSED( p );
		int samples = f->audioSamples(), channels = f->profile.getAudioChannels();
		float *in = (float*)src->data();
		float *out = (float*)dst->data();
		double vol = getParamValue( volume, f->pts() ).toDouble();
		double d = (double)samples * MICROSECOND / (double)f->profile.getAudioSampleRate();
		double vol2 = getParamValue( volume, f->pts() + d ).toDouble();
		d = (vol2 - vol) / samples;

		for ( int i = 0; i < samples; ++i ) {
			for ( int j = 0; j < channels; ++j )
				out[(i * channels) + j] = (double)in[(i * channels) + j] * vol;
			vol += d;
		}
		return true;
	}

protected:
	Parameter *volume;
};



class AudioFadeIn : public AudioVolume
{
public:
	AudioFadeIn( QString id, QString name ) : AudioVolume( id, name ) {
		setSnap( SNAPSTART );
		setLength( MICROSECOND * 2 );
		double one = volume->defValue.toDouble() / volume->max.toDouble();
		volume->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 0 ) );
		volume->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, one ) );
		volume->hidden = true;
	}
};



class AudioFadeOut : public AudioVolume
{
public:
	AudioFadeOut( QString id, QString name ) : AudioVolume( id, name ) {
		setSnap( SNAPEND );
		setLength( MICROSECOND * 2 );
		double one = volume->defValue.toDouble() / volume->max.toDouble();
		volume->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, one ) );
		volume->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 0 ) );
		volume->hidden = true;
	}
};

#endif //AUDIOVOLUME_H
