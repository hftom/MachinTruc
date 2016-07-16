#ifndef TRANSITION_H
#define TRANSITION_H

#include "vfx/glfilter.h"
#include "afx/audiofilter.h"



class Transition
{
public:
	Transition( double posInTrackPTS, double len );
	Transition(Transition *trans);

	void setVideoFilter( QSharedPointer<GLFilter> f );
	QSharedPointer<GLFilter> getVideoFilter() { return videoFilter; }
	void setAudioFilter( QSharedPointer<AudioFilter> f );
	QSharedPointer<AudioFilter> getAudioFilter() { return audioFilter; }
	
	double position() { return posInTrack; }
	void setPosition( double p );
	double length();
	void setLength( double len );
	
	void setFrameDuration( double d ) { frameDuration = d; }
	double getFrameDuration() {return frameDuration;}

private:
	QSharedPointer<GLFilter> videoFilter;
	QSharedPointer<AudioFilter> audioFilter;

	double posInTrack;
	double transitionLength;
	double frameDuration;
};

#endif //TRANSITION_H