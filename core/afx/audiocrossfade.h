#ifndef AUDIOCROSSFADE_H
#define AUDIOCROSSFADE_H

#include "audiovolume.h"



class AudioCrossFade : public AudioFilter
{
public:
	AudioCrossFade( QString id, QString name ) : AudioFilter( id, name ) {
		firstVol = new AudioVolume( "AudioVolume", "AudioVolume" );
		Parameter* p = firstVol->getParameters().first();
		p->hidden = true;
		double one = p->defValue.toDouble() / p->max.toDouble();
		p->name = tr( "Volume A:" );
		p->id = "volumeA";
		p->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, one ) );
		p->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, 0 ) );
		
		secondVol = new AudioVolume( "AudioVolume", "AudioVolume" );
		p = secondVol->getParameters().first();
		p->hidden = true;
		p->name = tr( "Volume B:" );
		p->id = "volumeB";
		p->graph.keys.append( AnimationKey( AnimationKey::CURVE, 0, 0 ) );
		p->graph.keys.append( AnimationKey( AnimationKey::CURVE, 1, one ) );
	}

	~AudioCrossFade() {
		delete firstVol;
		delete secondVol;
	}

	bool process( Frame *first, Buffer *buf1, Frame *second, Buffer *buf2, Buffer *dst, Profile *p ) {
		// ATTENTION: first or second can be NULL (but not both)
		// a NULL frame is considered silent.
		int samples, channels;
		if ( first ) {
			firstVol->process( first, buf1, buf1, p );
			samples = first->audioSamples();
			channels = first->profile.getAudioChannels();
		}
		if ( second ) {
			secondVol->process( second, buf2, buf2, p );
			if ( !first ) {
				samples = second->audioSamples();
				channels = second->profile.getAudioChannels();
			}
		}

		if ( first && second ) {
			float *in = (float*)buf1->data();
			float *in2 = (float*)buf2->data();
			float *out = (float*)dst->data();

			for ( int i = 0; i < samples; ++i ) {
				for ( int j = 0; j < channels; ++j )
					out[(i * channels) + j] = in[(i * channels) + j] + in2[(i * channels) + j];
			}
		}
		else if ( first ) {
			int bps = first->profile.getAudioChannels() * Profile::bytesPerChannel( &first->profile );
			memcpy( dst->data(), buf1->data(), samples * bps );
		}
		else {
			int bps = second->profile.getAudioChannels() * Profile::bytesPerChannel( &second->profile );
			memcpy( dst->data(), buf2->data(), samples * bps );
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
