#ifndef TRACK_H
#define TRACK_H

#include <QList>

#include "vfx/glfilter.h"
#include "input/input.h"
#include "engine/clip.h"



class Track
{
public:
	Track();
	~Track();

	bool insertClip( Clip *c );
	bool insertClipAt( Clip *c, int idx );
	int indexOf( Clip *c );
	Clip* removeClip( int idx );
	bool removeClip( Clip *c );
	int clipCount();
	Clip* clipAt( int i );
	int currentClipIndex();
	void setCurrentClipIndex( int i );
	int currentClipIndexAudio();
	void setCurrentClipIndexAudio( int i );

	void resetIndexes();

private:
	QList<Clip*> clips;
	int clipIndex, clipIndexAudio;
};
#endif //TRACK_H
