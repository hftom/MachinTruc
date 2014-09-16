#include <QList>

#include "engine/track.h"



Track::Track()
	: clipIndex( 0 ),
	clipIndexAudio( 0 ),
	transitionIndex( 0 ),
	transitionIndexAudio( 0 )
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



bool Track::insertTransition( Transition *t )
{
	transitions.append( t );
	return true;
}



bool Track::removeTransition( Transition *t )
{
	return transitions.removeOne( t );
}



int Track::transitionCount()
{
	return transitions.count();
}



Transition* Track::transitionAt( int i )
{
	return transitions.at( i );
}



int Track::currentTransitionIndex()
{
	return transitionIndex;
}



void Track::setCurrentTransitionIndex( int i )
{
	transitionIndex = i;
}



int Track::currentTransitionIndexAudio()
{
	return transitionIndexAudio;
}



void Track::setCurrentTransitionIndexAudio( int i )
{
	transitionIndexAudio = i;
}


void Track::resetIndexes()

{
	clipIndex = clipIndexAudio = transitionIndex = transitionIndexAudio = 0;
}
