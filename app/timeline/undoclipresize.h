#ifndef UNDO_CLIP_RESIZE_H
#define UNDO_CLIP_RESIZE_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoClipResize : public QUndoCommand
{
public:
	UndoClipResize(Timeline *t, Clip *c, bool start, int track, double oldPos, double newPos, double oldLen, double newLen,
				 Transition *oldTrans, Transition *newTrans, bool alreadyResized) {
		clip = c;
		timeline = t;
		oldTransition = oldTrans ? new Transition(oldTrans) : NULL;
		newTransition = newTrans ? new Transition(newTrans) : NULL;
		trackNumber = track;
		oldPosition = oldPos;
		newPosition = newPos;
		oldLength = oldLen;
		newLength = newLen;
		firstRedo = alreadyResized;
		resizeStart = start;
		setText(QObject::tr("Resize clip"));
	}
	
	~UndoClipResize() {
		if (oldTransition) delete oldTransition;
		if (newTransition) delete newTransition;;
	}
	
	void redo() {
		if (firstRedo) {
			firstRedo = false;
		}
		else {
			timeline->commandResizeClip(clip, resizeStart, trackNumber, newPosition, newLength, newTransition);
		}
	}
	
	void undo() {
		timeline->commandResizeClip(clip, resizeStart, trackNumber, oldPosition, oldLength, oldTransition);
	}
	
private:
	bool firstRedo, resizeStart;
	int trackNumber;
	double oldPosition, newPosition, oldLength, newLength;
	Clip *clip;
	Timeline *timeline;
	Transition *oldTransition, *newTransition;
};

#endif // UNDO_CLIP_RESIZE_H
