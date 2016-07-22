#ifndef UNDO_EFFECT_ADD_H
#define UNDO_EFFECT_ADD_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoEffectAdd : public QUndoCommand
{
public:
	UndoEffectAdd(Timeline *t, Clip *c, int track, QSharedPointer<Filter> f, bool isVideo, int index) {
		clip = c;
		timeline = t;
		trackNumber = track;
		filter = f;
		isVideoEffect = isVideo;
		indexInEffectList = index;
		setText(QObject::tr("Add %1 effect").arg(f->getFilterName()));
	}
	
	~UndoEffectAdd() {
	}
	
	void redo() {
		timeline->commandEffectAddRemove(clip, trackNumber, filter, isVideoEffect, indexInEffectList, false);
	}
	
	void undo() {
		timeline->commandEffectAddRemove(clip, trackNumber, filter, isVideoEffect, indexInEffectList, true);
	}
	
private:
	int trackNumber;
	Clip *clip;
	Timeline *timeline;
	QSharedPointer<Filter> filter;
	bool isVideoEffect;
	int indexInEffectList;
};

#endif // UNDO_EFFECT_ADD_H
