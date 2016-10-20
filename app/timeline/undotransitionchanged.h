#ifndef UNDO_TRANSITION_CHANGED_H
#define UNDO_TRANSITION_CHANGED_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoTransitionChanged : public QUndoCommand
{
public:
	UndoTransitionChanged(Timeline *t, Clip *c, QSharedPointer<Filter> old, QString filter, bool isVideo) {
		timeline = t;
		clip = c;
		oldFilter = old;
		newFilter = filter;
		isVideoEffect = isVideo;
	}
	
	void redo() {
		timeline->commandTransitionChanged(clip, oldFilter, newFilter, isVideoEffect, false);
	}
	
	void undo() {
		timeline->commandTransitionChanged(clip, oldFilter, newFilter, isVideoEffect, true);
	}
	
private:
	Timeline *timeline;
	Clip* clip;
	QSharedPointer<Filter> oldFilter;
	QString newFilter;
	bool isVideoEffect;
};

#endif // UNDO_TRANSITION_CHANGED_H
