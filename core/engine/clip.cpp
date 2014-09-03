#include "engine/clip.h"
#include "engine/util.h"



Clip::Clip( Source *src, double posInTrackPTS, double strt, double len )
	: source( src ),
	posInTrack( posInTrackPTS ),
	clipStart( strt ),
	clipLength( len ),
	frameDuration( MICROSECOND / 25.0 ),
	in( NULL )
{
}



Clip::~Clip()
{
	if ( in )
		in->setUsed( false );
}



void Clip::setPosition( double p )
{
	posInTrack = p;
	
	int i;
	for ( i = 0; i< videoFilters.count(); ++i )
		videoFilters.at( i )->setPosition( p );
}



void Clip::setLength( double len )
{
	clipLength = len;
	
	int i;
	for ( i = 0; i< videoFilters.count(); ++i )
		videoFilters.at( i )->setLength( length() );
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
