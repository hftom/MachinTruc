#include "engine/clip.h"
#include "engine/util.h"



Clip::Clip( Source *src, double posInTrackPTS, double strt, double len )
{
	source = src;
	posInTrack = posInTrackPTS;
	clipStart = strt;
	clipLength = len;
	frameDuration = MICROSECOND / 25.0;
	in = NULL;
}



Clip::~Clip()
{
	if ( in )
		in->setUsed( false );
}



double Clip::length()
{
	return nearestPTS( clipLength, frameDuration );
}



void Clip::setInput( InputBase *i )
{
	if ( in && i != in )
		in->setUsed( false );
	in = i;
	if ( in )
		in->setUsed( true );
}
