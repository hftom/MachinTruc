#include <QList>

#include "engine/track.h"



Track::Track()
{
	clipIndex = clipIndexAudio = 0;
	compositionIndex = 0;
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



bool Track::insertComposition( GLComposition *c )
{
	compositions.append( c );
	return true;
}
    


GLComposition* Track::removeComposition()
{
	return NULL;
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



int Track::compositionCount()
{
	return compositions.count();
}
    


GLComposition* Track::compositionAt( int i )
{
	return compositions.at( i );
}
    


int Track::currentCompositionIndex()
{
	return compositionIndex;
}
    


void Track::setCurrentCompositionIndex( int i )
{
	compositionIndex = i;
}


void Track::resetIndexes()

{
	clipIndex = clipIndexAudio = compositionIndex = 0;
}
