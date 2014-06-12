#ifndef TRACK_H
#define TRACK_H

#include <QList>

#include "vfx/glfilter.h"
#include "vfx/glcomposition.h"
#include "input/input.h"
#include "engine/clip.h"



class Track
{
public:
	Track();
	~Track();

	bool insertClip( Clip *c );
	bool insertClipAt( Clip *c, int idx );
	Clip* removeClip( int idx );
	bool removeClip( Clip *c );
	bool insertComposition( GLComposition *c );
	GLComposition* removeComposition();
	int clipCount();
	Clip* clipAt( int i );
	int currentClipIndex();
	void setCurrentClipIndex( int i );
	int currentClipIndexAudio();
	void setCurrentClipIndexAudio( int i );
	int compositionCount();
	GLComposition* compositionAt( int i );
	int currentCompositionIndex();
	void setCurrentCompositionIndex( int i );
	void resetIndexes();

private:
	QList<Clip*> clips;
	QList<GLComposition*> compositions;
	int clipIndex, clipIndexAudio, compositionIndex;
};
#endif //TRACK_H
