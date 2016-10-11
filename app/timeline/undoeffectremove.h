#ifndef UNDO_EFFECT_REMOVE_H
#define UNDO_EFFECT_REMOVE_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoEffectRemove : public QUndoCommand
{
public:
	UndoEffectRemove(Timeline *t) {
		timeline = t;
	}
	
	void append(Clip *c, int track, QSharedPointer<Filter> f, int index, bool isVideo) {
		clips.append(c);
		trackNumbers.append(track);
		filters.append(f);
		indexInEffectList.append(index);
		isVideoEffect = isVideo;
		
		if (clips.count() == 1) {
			setText(QObject::tr("Remove %1 effect").arg(f->getFilterName()));
		}
	}
	
	~UndoEffectRemove() {
	}
	
	void redo() {
		timeline->commandEffectAddRemove(clips, trackNumbers, filters, isVideoEffect, indexInEffectList, true);
	}
	
	void undo() {
		timeline->commandEffectAddRemove(clips, trackNumbers, filters, isVideoEffect, indexInEffectList, false);
	}
	
private:
	QList<int> trackNumbers;
	QList<Clip*> clips;
	Timeline *timeline;
	QList< QSharedPointer<Filter> > filters;
	bool isVideoEffect;
	QList<int> indexInEffectList;
};

#endif // UNDO_EFFECT_REMOVE_H
