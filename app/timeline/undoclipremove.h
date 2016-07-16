#ifndef UNDO_CLIP_REMOVE_H
#define UNDO_CLIP_REMOVE_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoClipRemove : public QUndoCommand
{
public:
	UndoClipRemove(Timeline *t, Clip *c, int track, Transition *tail) {
		clip = c;
		timeline = t;
		if (tail) {
			tailTransition = new Transition(*tail);
		}
		else {
			tailTransition = NULL;
		}
		trackNumber = track;
		redoState = true;
		setText(QObject::tr("Remove clip"));
	}
	
	~UndoClipRemove() {
		if (redoState) {
			delete clip;
		}
		if (tailTransition) {
			delete tailTransition;
		}
	}
	
	void redo() {
		timeline->commandRemoveClip(clip, trackNumber);
		redoState = true;
	}
	
	void undo() {
		timeline->commandAddClip(clip, trackNumber, tailTransition);
		redoState = false;
	}
	
private:
	bool redoState;
	int trackNumber;
	Clip *clip;
	Timeline *timeline;
	Transition *tailTransition;
};

#endif // UNDO_CLIP_REMOVE_H
