#ifndef UNDO_EFFECT_REORDER_H
#define UNDO_EFFECT_REORDER_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoEffectReorder : public QUndoCommand
{
public:
	UndoEffectReorder(Timeline *t, Clip *c, int track, int oldId, int newId, bool isVideo) {
		clip = c;
		timeline = t;
		video = isVideo;
		oldIndex = oldId;
		newIndex = newId;
		trackNumber = track;
		setText(QObject::tr("Reorder effect"));
	}
	
	void redo() {
		timeline->commandEffectReorder(clip, trackNumber, oldIndex, newIndex, video);
	}
	
	void undo() {
		timeline->commandEffectReorder(clip, trackNumber, newIndex, oldIndex, video);
	}
	
private:
	Clip *clip;
	Timeline *timeline;
	bool video;
	int oldIndex, newIndex;
	int trackNumber;
};

#endif // UNDO_EFFECT_REORDER_H
