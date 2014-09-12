#include "transition.h"



Transition::Transition( double posInTrackPTS, double len )
	: posInTrack( posInTrackPTS ),
	transitionLength( len )
{
	videoFilter = FilterCollection::getGlobalInstance()->videoTransitions.first().create().staticCast<GLFilter>();
	audioFilter = FilterCollection::getGlobalInstance()->audioTransitions.first().create().staticCast<AudioFilter>();
}



Transition::~Transition()
{
}



void Transition::setVideoFilter( GLFilter *f )
{
}



void Transition::setAudioFilter( AudioFilter *f )
{
}



void Transition::setPosition( double p )
{
}



double Transition::length()
{
}



void Transition::setLength( double len )
{
}
