#ifndef UNDO_CLIP_MOVE_H
#define UNDO_CLIP_MOVE_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoClipMove : public QUndoCommand
{
public:
	UndoClipMove(Timeline *t, Clip *c, bool multi, int oldTrack, int newTrack, double oldPos, double newPos,
				 Transition *oldTrans, Transition *newTrans, Transition *oldTail, Transition *newTail, bool alreadyMoved) {
		clip = c;
		timeline = t;
		oldTransition = oldTrans ? new Transition(oldTrans) : NULL;
		newTransition = newTrans ? new Transition(newTrans) : NULL;
		oldTailTransition = oldTail ? new Transition(oldTail) : NULL;
		newTailTransition = newTail ? new Transition(newTail) : NULL;
		oldTrackNumber = oldTrack;
		newTrackNumber = newTrack;
		oldPosition = oldPos;
		newPosition = newPos;
		firstRedo = alreadyMoved;
		multiMove = multi;
		setText(multi ? QObject::tr("Move clip's tail") : QObject::tr("Move clip"));
	}
	
	~UndoClipMove() {
		if (oldTransition) delete oldTransition;
		if (newTransition) delete newTransition;
		if (oldTailTransition) delete oldTailTransition;
		if (newTailTransition) delete newTailTransition;
	}
	
	void redo() {
		if (firstRedo) {
			firstRedo = false;
		}
		else {
			timeline->commandMoveClip(clip, multiMove, oldTrackNumber, newTrackNumber, newPosition, newTransition, newTailTransition);
		}
	}
	
	void undo() {
		timeline->commandMoveClip(clip, multiMove, newTrackNumber, oldTrackNumber, oldPosition, oldTransition, oldTailTransition);
	}
	
private:
	bool firstRedo, multiMove;
	int oldTrackNumber, newTrackNumber;
	double oldPosition, newPosition;
	Clip *clip;
	Timeline *timeline;
	Transition *oldTransition, *newTransition, *oldTailTransition, *newTailTransition;
};

#endif // UNDO_CLIP_MOVE_H
