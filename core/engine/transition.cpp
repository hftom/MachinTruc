#include "filtercollection.h"

#include "transition.h"



Transition::Transition( double posInTrackPTS, double len )
	: posInTrack( posInTrackPTS ),
	transitionLength( len )
{
	videoFilter = FilterCollection::getGlobalInstance()->videoTransitions.first().create().staticCast<GLFilter>();
	videoFilter->setPosition( posInTrack );
	videoFilter->setLength( transitionLength );
	
	audioFilter = FilterCollection::getGlobalInstance()->audioTransitions.first().create().staticCast<AudioFilter>();
	audioFilter->setPosition( posInTrack );
	audioFilter->setLength( transitionLength );
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
	return transitionLength;
}



void Transition::setLength( double len )
{
}
