#ifndef TRACK_H
#define TRACK_H

#include <QList>

#include "vfx/glfilter.h"
#include "input/input.h"
#include "engine/clip.h"
#include "engine/transition.h"



class Track
{
public:
	Track();
	~Track();

	bool insertClip( Clip *c );
	bool insertClipAt( Clip *c, int idx );
	Clip* removeClip( int idx );
	bool removeClip( Clip *c );
	int clipCount();
	Clip* clipAt( int i );
	int currentClipIndex();
	void setCurrentClipIndex( int i );
	int currentClipIndexAudio();
	void setCurrentClipIndexAudio( int i );
	
	bool insertTransition( Transition *t );
	bool removeTransition( Transition *t );
	int transitionCount();
	Transition* transitionAt( int i );
	int currentTransitionIndex();
	void setCurrentTransitionIndex( int i );
	int currentTransitionIndexAudio();
	void setCurrentTransitionIndexAudio( int i );
	void resetIndexes();

private:
	QList<Clip*> clips;
	QList<Transition*> transitions;
	int clipIndex, clipIndexAudio, transitionIndex, transitionIndexAudio;
};
#endif //TRACK_H
