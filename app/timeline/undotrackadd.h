#ifndef UNDO_TRACK_ADD_H
#define UNDO_TRACK_ADD_H

#include "undo.h"
#include "timeline.h"



class UndoTrackAdd : public QUndoCommand
{
public:
	UndoTrackAdd(Timeline *t, int trackIndex, bool remove) {
		timeline = t;
		index = trackIndex;
		rm = remove;
		setText(rm ? QObject::tr("Remove track") : QObject::tr("Add track"));
	}
	
	void redo() {
		timeline->commandTrackAddRemove(index, rm);
	}
	
	void undo() {
		timeline->commandTrackAddRemove(index, !rm);
	}
	
private:
	int index;
	bool rm;
	Timeline *timeline;
};

#endif // UNDO_TRACK_ADD_H
