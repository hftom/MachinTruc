#ifndef XMLIZER_H
#define XMLIZER_H

#include <QtXml>

#include "engine/scene.h"
#include "engine/filtercollection.h"
#include "shadercollection.h"



class XMLizer
{
public:
	static Clip* readClip( QDomElement &element, QList<Source*> *sourcesList, QList<Source*> *builtin, Scene *scene, bool &readError );
	static void readTransition( QDomElement &element, Clip *clip, bool &readError );
	static QSharedPointer<Filter> readFilter( QDomElement &element, bool audio, bool &readError, bool transition = false );
	static void readParameter( QDomElement &element, QSharedPointer<Filter> f );

	static void writeClip( QDomDocument &document, QDomNode &parent, Clip *clip );
	static void writeFilter( QDomDocument &document, QDomNode &parent, bool audio, QSharedPointer<Filter> f );
	
	static void createText( QDomDocument &document, QDomNode &parent, QString name, QString val );
	static void createInt( QDomDocument &document, QDomNode &parent, QString name, int val );
	static void createInt64( QDomDocument &document, QDomNode &parent, QString name, qint64 val );
	static void createDouble( QDomDocument &document, QDomNode &parent, QString name, double val );
};

#endif //XMLIZER_H
