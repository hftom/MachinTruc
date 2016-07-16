#ifndef UNDO_CLIP_ADD_H
#define UNDO_CLIP_ADD_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoClipAdd : public QUndoCommand
{
public:
	UndoClipAdd(Timeline *t, Clip *c, int track, Transition *tail, bool alreadyAdded) {
		clip = c;
		timeline = t;
		if (tail) {
			tailTransition = new Transition(*tail);
		}
		else {
			tailTransition = NULL;
		}
		trackNumber = track;
		firstRedo = alreadyAdded;
		redoState = true;
		setText(QObject::tr("Add clip"));
	}
	
	~UndoClipAdd() {
		if (!redoState) {
			delete clip;
		}
		if (tailTransition) {
			delete tailTransition;
		}
	}
	
	void redo() {
		if (firstRedo) {
			firstRedo = false;
		}
		else {
			timeline->commandAddClip(clip, trackNumber, tailTransition);
		}
		redoState = true;
	}
	
	void undo() {
		timeline->commandRemoveClip(clip, trackNumber);
		redoState = false;
	}
	
private:
	bool firstRedo;
	bool redoState;
	int trackNumber;
	Clip *clip;
	Timeline *timeline;
	Transition *tailTransition;
};

#endif // UNDO_CLIP_ADD_H
