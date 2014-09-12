#include "transition.h"



Transition::Transition( double posInTrackPTS, double len )
	: posInTrack( posInTrackPTS ),
	transitionLength( len )
{
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
