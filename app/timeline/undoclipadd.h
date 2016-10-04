#ifndef UNDO_CLIP_ADD_H
#define UNDO_CLIP_ADD_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoClipAdd : public QUndoCommand
{
public:
	UndoClipAdd(Timeline *t, Clip *c, int track, Transition *tail, bool alreadyAdded) {
		timeline = t;
		firstRedo = alreadyAdded;
		redoState = true;
		append(c, track, tail);
	}
	
	void append(Clip *c, int track, Transition *tail) {
		trackNumbers.append(track);
		clips.append(c);
		tailTransitions.append(tail ? new Transition(tail) : NULL);
		if (clips.count() == 1) {
			setText(QObject::tr("Add clip"));
		}
		else if (clips.count() == 2) {
			setText(QObject::tr("Add clips"));
		}
	}
	
	~UndoClipAdd() {
		if (!redoState) {
			while (clips.count()) 
				delete clips.takeFirst();
		}
		while (tailTransitions.count()) 
			delete tailTransitions.takeFirst();
	}
	
	void redo() {
		if (firstRedo) {
			firstRedo = false;
		}
		else {
			timeline->commandAddClip(clips, trackNumbers, tailTransitions);
		}
		redoState = true;
	}
	
	void undo() {
		timeline->commandRemoveClip(clips, trackNumbers);
		redoState = false;
	}
	
private:
	bool firstRedo;
	bool redoState;
	Timeline *timeline;
	QList<int> trackNumbers;
	QList<Clip*> clips;
	QList<Transition*> tailTransitions;
};

#endif // UNDO_CLIP_ADD_H
