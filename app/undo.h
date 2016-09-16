#ifndef UNDO_H
#define UNDO_H

#include <QUndoStack>
#include <QUndoCommand>
#include <QDebug>

#define UNDO_EFFECT_PARAM 1



class UndoStack : public QUndoStack
{
	Q_OBJECT
public:
	static UndoStack* getStack() {
		return &stack;
	}
	
public slots:	
	void redo() {
		//qDebug() << "Redo" << command(index());
		QUndoStack::redo();
	}
	
	void undo() {
		//qDebug() << "Undo" << command(index() - 1);
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
