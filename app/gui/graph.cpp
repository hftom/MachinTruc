#include <QTimer>
#include <QMenu>

#include "engine/filtercollection.h"
#include "grapheffectitem.h"

#define ITEMSPACING 10



Graph::Graph( bool audio )
	: isAudio( audio ),
	viewHeight( 0 ),
	currentClip( NULL ),
	selectedItem( NULL )
{
	setBackgroundBrush( QBrush( QColor(255,220,166) ) );
}



void Graph::viewSizeChanged( const QSize &size )
{
	viewHeight = size.height();
	QTimer::singleShot ( 1, this, SLOT(updateLength()) );
}



void Graph::rebuildGraph()
{
	setCurrentClip( currentClip );
}



void Graph::setCurrentClip( Clip *c )
{
	QList<QGraphicsItem*> list = items();
	while ( !list.isEmpty() ) {
		QGraphicsItem *it = list.takeFirst();
		removeItem( it );
		delete it;
	}
	selectedItem = NULL;
	currentClip = c;
	if ( !currentClip ) {
		emit filterSelected( NULL, 0 );
		return;
	}
	
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	addRect( 0, 5, ICONSIZEWIDTH, ICONSIZEHEIGHT, QPen(QColor("red")), QBrush(QColor("black")) );
	int y = ITEMSPACING * 3 / 2 + ICONSIZEHEIGHT;
	GraphEffectItem *it = NULL;
	int index = currentClip->videoFilters.currentIndex();
	for ( int i = 0; i < currentClip->videoFilters.count(); ++i ) {
		QString icon = "lens";
		QString name = "";
		QString id = currentClip->videoFilters.at( i )->getIdentifier();
		for ( int j = 0; j < fc->videoFilters.count(); ++j ) {
			if ( fc->videoFilters[ j ].identifier == id ) {
				icon = fc->videoFilters[ j ].icon;
				name = fc->videoFilters[ j ].name;
				break;
			}
		}
		GraphEffectItem *g = new GraphEffectItem( name, icon, i );
		if ( i == index )
			it = g;
		addItem( g );
		g->setPos( 0, y );
		y += ICONSIZEHEIGHT + ITEMSPACING;
	}
	
	itemSelected( it );
	QTimer::singleShot ( 1, this, SLOT(updateLength()) );
}



void Graph::itemSelected( GraphEffectItem *it )
{
	if ( selectedItem )
		selectedItem->setSelected( false );
	if ( it ) {
		it->setSelected (true );
		selectedItem = it;
		emit filterSelected( currentClip, currentClip->videoFilters.setCurrentIndex( it->index() ) );
	}
	else {
		selectedItem = NULL;
		emit filterSelected( currentClip, -1 );
	}
}



void Graph::effectRightClick( GraphEffectItem *it )
{
	QMenu menu;
	menu.addAction( tr("Delete") );
	QAction *action = menu.exec( QCursor::pos() );
	if ( !action )
		return;
	
	emit filterDeleted( currentClip, currentClip->videoFilters.at( it->index() ) );
	QTimer::singleShot ( 1, this, SLOT(rebuildGraph()) );
}



void Graph::updateLength()
{
	int maxlen = 0;
	QList<QGraphicsItem*> list = items();
	
	for ( int i = 0; i < list.count(); ++i ) {
		QGraphicsRectItem *it = (QGraphicsRectItem*)list.at( i );
		if ( it->pos().y() + it->rect().height() > maxlen )
			maxlen = it->pos().y() + it->rect().height();
	}
	
	maxlen += ICONSIZEHEIGHT;
	if ( maxlen < viewHeight )
		maxlen = viewHeight;
	
	setSceneRect( 0, 0, ICONSIZEWIDTH, maxlen );
}
