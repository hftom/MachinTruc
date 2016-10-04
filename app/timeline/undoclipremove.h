#ifndef UNDO_CLIP_REMOVE_H
#define UNDO_CLIP_REMOVE_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoClipRemove : public QUndoCommand
{
public:
	UndoClipRemove(Timeline *t) {
		timeline = t;
		redoState = true;
	}
	
	void append(Clip *c, int track, Transition *tail) {
		trackNumbers.append(track);
		clips.append(c);
		tailTransitions.append(tail ? new Transition(tail) : NULL);
		if (clips.count() == 1) {
			setText(QObject::tr("Remove clip"));
		}
		else if (clips.count() == 2) {
			setText(QObject::tr("Remove clips"));
		}
	}
	
	~UndoClipRemove() {
		if (redoState) {
			while (clips.count()) 
				delete clips.takeFirst();
		}
		while (tailTransitions.count()) 
			delete tailTransitions.takeFirst();
	}
	
	void redo() {
		timeline->commandRemoveClip(clips, trackNumbers);
		redoState = true;
	}
	
	void undo() {
		timeline->commandAddClip(clips, trackNumbers, tailTransitions);
		redoState = false;
	}
	
private:
	bool redoState;
	Timeline *timeline;
	QList<int> trackNumbers;
	QList<Clip*> clips;
	QList<Transition*> tailTransitions;
};

#endif // UNDO_CLIP_REMOVE_H
