#ifndef UNDO_EFFECT_RESIZE_H
#define UNDO_EFFECT_RESIZE_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoEffectResize : public QUndoCommand
{
public:
	UndoEffectResize(Timeline *t, Clip *c, bool start, double oldOffset, double newOffset, double pos, double oldLen, double newLen, bool isVideo, int index, bool alreadyResized) {
		clip = c;
		timeline = t;
		firstRedo = alreadyResized;
		position = pos;
		oldLength = oldLen;
		newLength = newLen;
		oldPositionOffset = oldOffset;
		newPositionOffset = newOffset;
		resizeStart = start;
		video = isVideo;
		effectIndex = index;
		setText(QObject::tr("Resize clip"));
	}
	
	void redo() {
		if (firstRedo) {
			firstRedo = false;
		}
		else {
			timeline->commandEffectResize(clip, resizeStart, newPositionOffset, position, newLength, video, effectIndex);
		}
	}
	
	void undo() {
		timeline->commandEffectResize(clip, resizeStart, oldPositionOffset, position, oldLength, video, effectIndex);
	}
	
private:
	Clip *clip;
	Timeline *timeline;
	double position;
	double oldPositionOffset, newPositionOffset;
	double oldLength, newLength;
	bool video, firstRedo, resizeStart;
	int effectIndex;
};

#endif // UNDO_EFFECT_RESIZE_H
