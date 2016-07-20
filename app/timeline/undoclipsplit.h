#ifndef UNDO_CLIP_SPLIT_H
#define UNDO_CLIP_SPLIT_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoClipSplit : public QUndoCommand
{
public:
	UndoClipSplit(Timeline *t, Clip *c, Clip *c1, Clip *c2, int track, Transition *trans, Transition *tail, bool alreadyDone) {
		clip = c;
		clip1 = c1;
		clip2 = c2;
		timeline = t;
		tailTransition = tail ? new Transition(tail) : NULL;
		transition = trans ? new Transition(trans) : NULL;
		trackNumber = track;
		firstRedo = alreadyDone;
		redoState = true;
		setText(QObject::tr("Split clip"));
	}
	
	~UndoClipSplit() {
		if (redoState) {
			delete clip;
		}
		else {
			delete clip1;
			delete clip2;
		}
		if (tailTransition) {
			delete tailTransition;
		}
		if (transition) {
			delete transition;
		}
	}
	
	void redo() {
		if (firstRedo) {
			firstRedo = false;
		}
		else {
			timeline->commandSplitClip(clip, clip1, clip2, trackNumber, transition, tailTransition, true);
		}
		redoState = true;
	}
	
	void undo() {
		timeline->commandSplitClip(clip, clip1, clip2, trackNumber, transition, tailTransition, false);
		redoState = false;
	}
	
private:
	bool firstRedo;
	bool redoState;
	int trackNumber;
	Clip *clip, *clip1, *clip2;
	Timeline *timeline;
	Transition *transition, *tailTransition;
};

#endif // UNDO_CLIP_SPLIT_H
