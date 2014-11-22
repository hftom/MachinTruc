#include <QList>

#include "engine/track.h"



Track::Track()
	: clipIndex( 0 ),
	clipIndexAudio( 0 )
{
}



Track::~Track()
{
}



bool Track::insertClip( Clip *c )
{
	clips.append( c );
	return true;
}



bool Track::insertClipAt( Clip *c, int idx )
{
	clips.insert( idx, c );
	return true;
}



int Track::indexOf( Clip *c )
{
	return clips.indexOf( c );
}



Clip* Track::removeClip( int idx ) 
{
	return clips.takeAt( idx );
}



bool Track::removeClip( Clip *c )
{
	return clips.removeOne( c );
}



int Track::clipCount()
{
	return clips.count();
}



Clip* Track::clipAt( int i )
{
	return clips.at( i );
}



int Track::currentClipIndex()
{
	return clipIndex;
}



void Track::setCurrentClipIndex( int i )
{
	clipIndex = i;
}



int Track::currentClipIndexAudio()
{
	return clipIndexAudio;
}



void Track::setCurrentClipIndexAudio( int i )
{
	clipIndexAudio = i;
}



void Track::resetIndexes( bool backward )

{
	if ( backward )
		clipIndex = clipIndexAudio = clips.count() - 1;
	else
		clipIndex = clipIndexAudio = 0;
}
