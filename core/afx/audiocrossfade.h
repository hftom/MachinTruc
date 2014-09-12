#ifndef AUDIOCROSSFADE_H
#define AUDIOCROSSFADE_H

#include "audiovolume.h"



class AudioCrossFade : public AudioFilter
{
public:
	AudioCrossFade( QString id, QString name ) : AudioFilter( id, name ) {
		first = new AudioVolume( "AudioVolume", "AudioVolume" );
		Parameter* p = first->getParameters().at( 0 );
		p->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 1 ) );
		p->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 0 ) );
		
		second = new AudioVolume( "AudioVolume", "AudioVolume" );
		p = second->getParameters().at( 0 );
		p->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 0 ) );
		p->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 1 ) );
	}

	~AudioCrossFade() {
		delete first;
		delete second;
	}

	bool process( Frame *src, Frame *dst, Profile *p ) {
		first->process( src, p );
		second->process( dst, p );
		
		int samples = src->audioSamples(), channels = src->profile.getAudioChannels();
		int16_t *in = (int16_t*)src->data();
		int16_t *out = (int16_t*)dst->data();

		for ( int i = 0; i < samples; ++i ) {
			for ( int j = 0; j < channels; ++j )
				out[(i * channels) + j] += in[(i * channels) + j];
		}
		return true;
	}

	// Filter virtuals
	void setPosition( double p ) {
		Filter::setPosition( p );
		first->setPosition( p );
		second->setPosition( p );
	}

	void setLength( double len ) {
		Filter::setLength( len );
		first->setLength( len );
		second->setLength( len );
	}

	QList<Parameter*> getParameters() {
		return first->getParameters() + second->getParameters();
	}

private:
	AudioVolume *first, *second;
};

#endif //AUDIOCROSSFADE_H
