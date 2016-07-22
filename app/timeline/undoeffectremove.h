#ifndef UNDO_EFFECT_REMOVE_H
#define UNDO_EFFECT_REMOVE_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoEffectRemove : public QUndoCommand
{
public:
	UndoEffectRemove(Timeline *t, Clip *c, int track, QSharedPointer<Filter> f, bool isVideo, int index) {
		clip = c;
		timeline = t;
		trackNumber = track;
		filter = f;
		isVideoEffect = isVideo;
		indexInEffectList = index;
		setText(QObject::tr("Remove %1 effect").arg(f->getFilterName()));
	}
	
	~UndoEffectRemove() {
	}
	
	void redo() {
		timeline->commandEffectAddRemove(clip, trackNumber, filter, isVideoEffect, indexInEffectList, true);
	}
	
	void undo() {
		timeline->commandEffectAddRemove(clip, trackNumber, filter, isVideoEffect, indexInEffectList, false);
	}
	
private:
	int trackNumber;
	Clip *clip;
	Timeline *timeline;
	QSharedPointer<Filter> filter;
	bool isVideoEffect;
	int indexInEffectList;
};

#endif // UNDO_EFFECT_REMOVE_H
