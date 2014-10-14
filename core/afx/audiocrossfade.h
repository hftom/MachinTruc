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
		// ATTENTION: first or second can be NULL (but not both)
		// if first is NULL result goes in second->data()
		// else it goes in first->data()		
		int samples, channels;
		if ( first ) {
			firstVol->process( first, p );
			samples = first->audioSamples();
			channels = first->profile.getAudioChannels();
		}
		if ( second ) {
			secondVol->process( second, p );
			if ( !first ) {
				samples = second->audioSamples();
				channels = second->profile.getAudioChannels();
			}
		}

		if ( first && second ) {
			int16_t *in = (int16_t*)first->data();
			int16_t *out = (int16_t*)second->data();

			for ( int i = 0; i < samples; ++i ) {
				for ( int j = 0; j < channels; ++j )
					in[(i * channels) + j] += out[(i * channels) + j];
			}
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
