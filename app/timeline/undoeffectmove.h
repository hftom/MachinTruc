#ifndef UNDO_EFFECT_MOVE_H
#define UNDO_EFFECT_MOVE_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoEffectMove : public QUndoCommand
{
public:
	UndoEffectMove(Timeline *t, Clip *c, double oldPos, double newPos, bool isVideo, int index, bool alreadyMoved) {
		clip = c;
		timeline = t;
		firstRedo = alreadyMoved;
		oldPosition = oldPos;
		newPosition = newPos;
		video = isVideo;
		effectIndex = index;
		setText(QObject::tr("Move clip"));
	}
	
	void redo() {
		if (firstRedo) {
			firstRedo = false;
		}
		else {
			timeline->commandEffectMove(clip, newPosition, video, effectIndex);
		}
	}
	
	void undo() {
		timeline->commandEffectMove(clip, oldPosition, video, effectIndex);
	}
	
private:
	Clip *clip;
	Timeline *timeline;
	double oldPosition, newPosition;
	bool video, firstRedo;
	int effectIndex;
};

#endif // UNDO_EFFECT_MOVE_H
