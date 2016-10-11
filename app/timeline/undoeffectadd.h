#ifndef UNDO_EFFECT_ADD_H
#define UNDO_EFFECT_ADD_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoEffectAdd : public QUndoCommand
{
public:
	UndoEffectAdd(Timeline *t) {
		timeline = t;
	}
	
	void append(Clip *c, int track, QSharedPointer<Filter> f, int index, bool isVideo) {
		clips.append(c);
		trackNumbers.append(track);
		filters.append(f);
		indexInEffectList.append(index);
		isVideoEffect = isVideo;
		
		if (clips.count() == 1) {
			setText(QObject::tr("Add %1 effect").arg(f->getFilterName()));
		}
	}
	
	~UndoEffectAdd() {
	}
	
	void redo() {
		timeline->commandEffectAddRemove(clips, trackNumbers, filters, isVideoEffect, indexInEffectList, false);
	}
	
	void undo() {
		timeline->commandEffectAddRemove(clips, trackNumbers, filters, isVideoEffect, indexInEffectList, true);
	}
	
private:
	QList<int> trackNumbers;
	QList<Clip*> clips;
	Timeline *timeline;
	QList< QSharedPointer<Filter> > filters;
	bool isVideoEffect;
	QList<int> indexInEffectList;
};

#endif // UNDO_EFFECT_ADD_H
