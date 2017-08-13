#include "clipboard.h"



ClipBoard::ClipBoard(QAction *copy, QAction *cut, QAction *paste)
{
	actionCopy = copy;
	actionCut = cut;
	actionPaste = paste;

	reset();
}



void ClipBoard::reset()
{
	actionCopy->setEnabled(false);
	actionCut->setEnabled(false);
	actionPaste->setEnabled(false);

	document = QDomDocument();
}



void ClipBoard::clipSelected(ClipViewItem *cv)
{
	bool en = cv != NULL;

	actionCopy->setEnabled(en);
	actionCut->setEnabled(en);

	QString copyType = getCopyType();
	if ( copyType.startsWith("Track") ) {
		actionPaste->setEnabled(true);
	}
	else {
		actionPaste->setEnabled(en && copyType != "");
	}
}



void ClipBoard::copyFilter(QSharedPointer<Filter> f, bool audio)
{
	document = QDomDocument( "MachinTrucVideoEdit" );
	QDomElement root = document.createElement( "MachinTrucClipboard" );
	document.appendChild( root );
	XMLizer::writeFilter( document, root, audio, f );

	QDomNode proc = document.createProcessingInstruction( "xml","version=\"1.0\" encoding=\"UTF-8\"" );
	document.insertBefore( proc, document.firstChild() );

	/*QFile data( "/home/cris/copy.mct" );
	if ( data.open( QFile::WriteOnly | QFile::Truncate ) ) {
		QTextStream out( &data );
		document.save( out, 2 );
	}*/

	actionPaste->setEnabled(true);
}



void ClipBoard::copyClips( QList< QList<Clip*>* > *list )
{
	document = QDomDocument( "MachinTrucVideoEdit" );
	QDomElement root = document.createElement( "MachinTrucClipboard" );
	document.appendChild( root );

	for (int i = 0; i < list->count(); ++i) {
		QDomElement n1 = document.createElement( "Track" + QString::number( i ) );
		root.appendChild( n1 );
		QList<Clip*> *track = list->at(i);
		for ( int j = 0; j < track->count(); ++j ) {
			XMLizer::writeClip( document, n1, track->at(j) );
		}
	}

	QDomNode proc = document.createProcessingInstruction( "xml","version=\"1.0\" encoding=\"UTF-8\"" );
	document.insertBefore( proc, document.firstChild() );

	actionPaste->setEnabled(true);
}



QString ClipBoard::getCopyType()
{
	QDomElement rootElement = document.documentElement();
	QDomNodeList nodes = rootElement.childNodes();

	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;

		return e.tagName();
	}

	return "";
}



QSharedPointer<Filter> ClipBoard::getFilter()
{
	bool readError = false;
	QDomElement rootElement = document.documentElement();
	QDomNodeList nodes = rootElement.childNodes();

	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;

		QSharedPointer<Filter> f = XMLizer::readFilter( e, e.tagName() == "AudioFilter", readError );
		if ( !f.isNull() ) {
			return f;
		}
	}

	return QSharedPointer<Filter>();
}



QList< QList<Clip*>* >* ClipBoard::getClips( QList<Source*> sourcesList, Scene *scene )
{
	QDomElement rootElement = document.documentElement();
	QDomNodeList nodes = rootElement.childNodes();

	QList< QList<Clip*>* > *list = new QList< QList<Clip*>* >();

	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;

		if ( e.tagName().startsWith( "Track" ) ) {
			QList<Clip*> *track = new QList<Clip*>();
			readTrack( e, track, &sourcesList, scene );
			list->append(track);
		}
	}

	return list;
}



void ClipBoard::deleteClips(QList< QList<Clip*>* > *list)
{
	while (!list->isEmpty()) {
		QList<Clip*> *track = list->takeFirst();
		while (!track->isEmpty()) {
			delete track->takeFirst();
		}
		delete track;
	}
	delete list;
}



void ClipBoard::readTrack( QDomElement &element, QList<Clip*> *track, QList<Source*> *sourcesList, Scene *scene )
{
	bool readError = false;
	QDomNodeList nodes = element.childNodes();

	for ( int i = 0; i < nodes.count(); ++i ) {
		QDomElement e = nodes.at( i ).toElement();
		if ( e.isNull() )
			continue;

		if ( e.tagName() == "Clip" ) {
			Clip *clip = XMLizer::readClip( e, sourcesList, scene, readError );
			if (clip) {
				int k = 0, tc = track->count();
				// sort clips by position
				while ( k < tc ) {
					Clip *c = track->at( k );
					if ( clip->position() < c->position() ) {
						track->insert( k, clip );
						break;
					}
					++k;
				}
				if ( k == tc ) {
					track->append( clip );
				}
			}
		}
	}
}
