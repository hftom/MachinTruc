#ifndef UNDO_CLIP_SPEED_H
#define UNDO_CLIP_SPEED_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoClipSpeed : public QUndoCommand
{
public:
	UndoClipSpeed(Timeline *t, Clip *c, int track, double oldSpeed, double newSpeed, double oldLen, double newLen, Transition *oldTail, Transition *newTail, bool alreadySet) {
		clip = c;
		timeline = t;
		oldTailTransition = oldTail ? new Transition(oldTail) : NULL;
		newTailTransition = newTail ? new Transition(newTail) : NULL;
		trackNumber = track;
		oldClipSpeed = oldSpeed;
		newClipSpeed = newSpeed;
		oldLength = oldLen;
		newLength = newLen;
		firstRedo = alreadySet;
		setText(QObject::tr("Clip speed"));
	}
	
	~UndoClipSpeed() {
		if (oldTailTransition) delete oldTailTransition;
		if (newTailTransition) delete newTailTransition;;
	}
	
	void redo() {
		if (firstRedo) {
			firstRedo = false;
		}
		else {
			timeline->commandClipSpeed(clip, trackNumber, newClipSpeed, newLength, newTailTransition);
		}
	}
	
	void undo() {
		timeline->commandClipSpeed(clip, trackNumber, oldClipSpeed, oldLength, oldTailTransition);
	}
	
private:
	bool firstRedo;
	int trackNumber;
	double oldClipSpeed, newClipSpeed;
	double oldLength, newLength;
	Clip *clip;
	Timeline *timeline;
	Transition *oldTailTransition, *newTailTransition;
};

#endif // UNDO_CLIP_SPEED_H
