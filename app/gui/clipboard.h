#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QAction>

#include "timeline/clipviewitem.h"
#include "xmlizer.h"



class ClipBoard : public QObject
{
	Q_OBJECT
public:
	ClipBoard(QAction *copy, QAction *cut, QAction *paste);

	QString getCopyType();
	
	void copyClips( QList< QList<Clip*>* > *list );
	QList< QList<Clip*>* >* getClips( QList<Source*> sourcesList, QList<Source*> builtin, Scene *scene );
	void deleteClips(QList< QList<Clip*>* > *list);
	
	
	void copyFilter(QSharedPointer<Filter> f, bool audio);
	QSharedPointer<Filter> getFilter();
	
public slots:
	void clipSelected(ClipViewItem*);
	void reset();

private:
	void readTrack( QDomElement &element, QList<Clip*> *track, QList<Source*> *sourcesList, QList<Source*> *builtin, Scene *scene );
	
	QDomDocument document;
	
	QAction *actionCopy, *actionCut, *actionPaste;
};

#endif // CLIPBOARD_H
