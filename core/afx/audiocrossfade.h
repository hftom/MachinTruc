#ifndef AUDIOCROSSFADE_H
#define AUDIOCROSSFADE_H

#include "audiovolume.h"



class AudioCrossFade : public AudioFilter
{
public:
	AudioCrossFade( QString id, QString name ) : AudioFilter( id, name ) {
		firstVol = new AudioVolume( "AudioVolume", "AudioVolume" );
		Parameter* p = firstVol->getParameters().first();
		double one = p->defValue.toDouble() / p->max.toDouble();
		p->name = tr( "Volume A:" );
		p->id = "volumeA";
		p->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, one ) );
		p->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 0 ) );
		
		secondVol = new AudioVolume( "AudioVolume", "AudioVolume" );
		p = secondVol->getParameters().first();
		p->name = tr( "Volume B:" );
		p->id = "volumeB";
		p->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 0 ) );
		p->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, one ) );
	}

	~AudioCrossFade() {
		delete firstVol;
		delete secondVol;
	}

	bool process( Frame *first, Frame *second, Profile *p ) {
		firstVol->process( first, p );
		secondVol->process( second, p );
		
		int samples = first->audioSamples(), channels = first->profile.getAudioChannels();
		int16_t *in = (int16_t*)first->data();
		int16_t *out = (int16_t*)second->data();

		for ( int i = 0; i < samples; ++i ) {
			for ( int j = 0; j < channels; ++j )
				in[(i * channels) + j] += out[(i * channels) + j];
		}
		return true;
	}

	// Filter virtuals
	void setPosition( double p ) {
		Filter::setPosition( p );
		firstVol->setPosition( p );
		secondVol->setPosition( p );
	}

	void setLength( double len ) {
		Filter::setLength( len );
		firstVol->setLength( len );
		secondVol->setLength( len );
	}

	QList<Parameter*> getParameters() {
		return firstVol->getParameters() + secondVol->getParameters();
	}

private:
	AudioVolume *firstVol, *secondVol;
};

#endif //AUDIOCROSSFADE_H
