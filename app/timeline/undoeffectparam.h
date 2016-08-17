#ifndef UNDO_EFFECT_PARAM_H
#define UNDO_EFFECT_PARAM_H

#include "undo.h"
#include "timeline.h"

#include "engine/clip.h"



class UndoEffectParam : public QUndoCommand
{
public:
	UndoEffectParam(Timeline *t, QSharedPointer<Filter> f, Parameter *p, QVariant oldVal, QVariant newVal) {
		timeline = t;
		filter = f;
		param = p;
		oldValue = oldVal;
		newValue = newVal;
		setText(QObject::tr("%1 value").arg(param->name));
		firstRedo = true;
	}
	
	void redo() {
		if (firstRedo) {
			firstRedo = false;
		}
		else {
			timeline->commandEffectParam(filter, param, newValue);
		}
	}
	
	void undo() {
		timeline->commandEffectParam(filter, param, oldValue);
	}
	
	int id() const {
		return UNDO_EFFECT_PARAM;
	}
	
	bool mergeWith(const QUndoCommand *other) {
		if (other->id() != id())
			return false;
		
		newValue = static_cast<const UndoEffectParam*>(other)->getNewValue();
		return true;
	}

	QVariant getNewValue() const {
		return newValue;
	}
	
private:
	Timeline *timeline;
	QSharedPointer<Filter> filter;
	Parameter *param;
	QVariant oldValue, newValue;
	bool firstRedo;
};

#endif // UNDO_EFFECT_PARAM_H
