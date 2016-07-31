#ifndef UNDO_H
#define UNDO_H

#include <QUndoStack>
#include <QUndoCommand>
#include <QDebug>

#define UNDO_CLIP_ADD
#define UNDO_CLIP_SIZE_POSITION



class UndoStack : public QUndoStack
{
	Q_OBJECT
public:
	static UndoStack* getStack() {
		return &stack;
	}
	
public slots:	
	void redo() {
		qDebug() << "Redo" << command(index());
		QUndoStack::redo();
	}
	
	void undo() {
		qDebug() << "Undo" << command(index() - 1);
		QUndoStack::undo();
	}
	
private:
	Q_DISABLE_COPY(UndoStack)
	UndoStack() {
		setUndoLimit(100);
	}

	~UndoStack() {}
	
	static UndoStack stack;
};

#endif // UNDO_H
