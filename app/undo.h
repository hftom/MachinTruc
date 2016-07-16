#ifndef UNDO_H
#define UNDO_H

#include <QUndoStack>
#include <QUndoCommand>
#include <QDebug>

#define UNDO_CLIP_ADD
#define UNDO_CLIP_SIZE_POSITION



class UndoStack : public QUndoStack
{
public:
	static UndoStack* getStack() {
		return stack;
	}
	
private:
	UndoStack() {}
	~UndoStack() {}
	
	static UndoStack* stack;
};

#endif // UNDO_H
