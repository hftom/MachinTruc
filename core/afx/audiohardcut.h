#ifndef AUDIOHARDCUT_H
#define AUDIOHARDCUT_H

#include "audiofilter.h"



class AudioHardCut : public AudioFilter
{
public:
	AudioHardCut( QString id, QString name ) : AudioFilter( id, name ) {
		position = addParameter( "position", tr("Cut position:"), Parameter::PDOUBLE, 0.5, 0.0, 1.0, false );
	}

	bool process( Frame *first, Buffer *buf1, Frame *second, Buffer *buf2, Buffer *dst, Profile *p ) {
		// ATTENTION: first or second can be NULL (but not both)
		// a NULL frame is considered silent.
		Q_UNUSED( p );

		if ( first && second ) {
			int samples = first->audioSamples();
			if ( getNormalizedTime( first->pts() ) < getParamValue( position ).toDouble() ) {
				int bps = first->profile.getAudioChannels() * first->profile.bytesPerChannel( &first->profile );
				memcpy( dst->data(), buf1->data(), samples * bps );
			}
			else {
				int bps = second->profile.getAudioChannels() * second->profile.bytesPerChannel( &second->profile );
				memcpy( dst->data(), buf2->data(), samples * bps );
			}
		}
		else if ( first ) {
			int samples = first->audioSamples();
			int bps = first->profile.getAudioChannels() * first->profile.bytesPerChannel( &first->profile );
			memcpy( dst->data(), buf1->data(), samples * bps );
		}
		else {
			int samples = second->audioSamples();
			int bps = second->profile.getAudioChannels() * second->profile.bytesPerChannel( &second->profile );
			memcpy( dst->data(), buf2->data(), samples * bps );
		}

		return true;
	}
	
private:
	Parameter *position;
};

#endif //AUDIOHARDCUT_H
