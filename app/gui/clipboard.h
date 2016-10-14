#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "xmlizer.h"



class ClipBoard
{
public:
	QString getCopyType();
	
	void copyClips( QList< QList<Clip*>* > *list );
	QList< QList<Clip*>* >* getClips( QList<Source*> sourcesList, Scene *scene );
	void deleteClips(QList< QList<Clip*>* > *list);
	
	
	void copyFilter(QSharedPointer<Filter> f, bool audio);
	QSharedPointer<Filter> getFilter();

private:
	void readTrack( QDomElement &element, QList<Clip*> *track, QList<Source*> *sourcesList, Scene *scene );
	
	QDomDocument document;
};

#endif // CLIPBOARD_H
