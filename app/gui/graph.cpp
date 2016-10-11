#include <QTimer>
#include <QMenu>

#include "engine/filtercollection.h"
#include "gui/mimetypes.h"
#include "grapheffectitem.h"

#define ITEMSPACING 10.0



Graph::Graph( bool audio )
	: isAudio( audio ),
	viewHeight( 0 ),
	currentClip( NULL ),
	currentThumb( NULL ),
	selectedItem( NULL )
{
	if ( isAudio )
		setBackgroundBrush( QBrush( QColor(192,255,255) ) );
	else
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



void Graph::setCurrentClip( ClipViewItem *c )
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
	currentThumb = new GraphThumb( c->getStartThumb() );
	addItem( currentThumb );
	currentThumb->setPos( 0, ITEMSPACING / 2 );
	int y = ITEMSPACING * 3 / 2 + ICONSIZEHEIGHT;
	GraphEffectItem *it = NULL;
	
	if ( isAudio ) {
		int index = currentClip->getClip()->audioFilters.currentIndex();
		for ( int i = 0; i < currentClip->getClip()->audioFilters.count(); ++i ) {
			QString icon = "sound";
			QString name = currentClip->getClip()->audioFilters.at( i )->getFilterName();
			QString id = currentClip->getClip()->audioFilters.at( i )->getIdentifier();
			for ( int j = 0; j < fc->audioFilters.count(); ++j ) {
				if ( fc->audioFilters[ j ].identifier == id ) {
					icon = fc->audioFilters[ j ].icon;
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
	}
	else {
		int index = currentClip->getClip()->videoFilters.currentIndex();
		for ( int i = 0; i < currentClip->getClip()->videoFilters.count(); ++i ) {
			QString icon = "lens";
			QString name = currentClip->getClip()->videoFilters.at( i )->getFilterName();
			QString id = currentClip->getClip()->videoFilters.at( i )->getIdentifier();
			for ( int j = 0; j < fc->videoFilters.count(); ++j ) {
				if ( fc->videoFilters[ j ].identifier == id ) {
					icon = fc->videoFilters[ j ].icon;
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
	}
	
	itemSelected( it );
	QTimer::singleShot ( 1, this, SLOT(updateLength()) );
}



void Graph::reloadCurrentFilter()
{
	if ( selectedItem )
		setCurrentClip( currentClip );
}



void Graph::itemSelected( GraphEffectItem *it )
{
	if ( selectedItem )
		selectedItem->setSelected( false );
	if ( it ) {
		it->setSelected (true );
		selectedItem = it;
		if ( isAudio )
			emit filterSelected( currentClip->getClip(), currentClip->getClip()->audioFilters.setCurrentIndex( it->index() ) );
		else
			emit filterSelected( currentClip->getClip(), currentClip->getClip()->videoFilters.setCurrentIndex( it->index() ) );
	}
	else {
		selectedItem = NULL;
		emit filterSelected( currentClip->getClip(), -1 );
	}
	
	if ( currentThumb && currentThumb->isSelected() ) {
		currentThumb->setSelected( false );
		showEffect( -1 ); 
	}
}



void Graph::itemDoubleClicked()
{
	if ( !selectedItem )
		return;
	
	int id = selectedItem->index();
	if ( currentThumb->isSelected() && id == currentEffectIndex ) {
		currentThumb->setSelected( false );
		emit showEffect( -1 );
	}
	else {
		currentThumb->setSelected( true );
		emit showEffect( id );
	}
	currentEffectIndex = id;
}



void Graph::hiddenEffect()
{
	if ( currentThumb )
		currentThumb->setSelected( false );
}



void Graph::effectRightClick( GraphEffectItem *it )
{
	QMenu menu;
	QAction *show = menu.addAction( tr("Show in timeline") );
	QAction *copy = menu.addAction( tr("Copy") );
	QAction *del = menu.addAction( tr("Delete") );
	QAction *action = menu.exec( QCursor::pos() );
	if ( action == show ) {
		itemSelected( it );
		itemDoubleClicked();
	}
	else if ( action == del ) {
		if ( isAudio )
			emit filterDeleted( currentClip->getClip(), currentClip->getClip()->audioFilters.at( it->index() ) );
		else
			emit filterDeleted( currentClip->getClip(), currentClip->getClip()->videoFilters.at( it->index() ) );
		emit showEffect( -1 );
		QTimer::singleShot ( 1, this, SLOT(rebuildGraph()) );
	}
	else if ( action == copy ) {
		if ( isAudio ) {
			emit filterCopy(currentClip->getClip()->audioFilters.at( it->index() ), isAudio);
		}
		else {
			emit filterCopy(currentClip->getClip()->videoFilters.at( it->index() ), isAudio);
		}
	}
	
	delete show;
	delete copy;
	delete del;
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
	emit showVerticalScrollBar( maxlen > viewHeight );
		
}



void Graph::effectMoved( GraphEffectItem *it, qreal y )
{
	int i, j;

	QList<QGraphicsItem*> list = items();
	y = qMax( y, (ITEMSPACING * 3.0 / 2.0) + ICONSIZEHEIGHT );
	y = qMin( y, (ITEMSPACING  / 2.0) + (ICONSIZEHEIGHT + ITEMSPACING) * (list.count() - 1) ); 
	QList<GraphEffectItem*> dragList;
	for ( i = 0; i < list.count(); ++i ) {
		GraphItem *e = (GraphItem*)list.at(i);
		if ( e->isEffect && e != it ) {
			for ( j = 0; j < dragList.count(); ++j ) {
				if ( e->y() < dragList.at(j)->y() ) {
					dragList.insert( j, (GraphEffectItem*)e );
					break;
				}
			}
			if ( j == dragList.count() )
				dragList.append( (GraphEffectItem*)e );
		}
	}

	for ( i = 0; i < dragList.count(); ++i ) {
		GraphEffectItem *g = dragList.at( i );
		qreal center = (ITEMSPACING * 3.0 / 2.0) + ICONSIZEHEIGHT + (i * (ICONSIZEHEIGHT + ITEMSPACING));
		qreal offset = qAbs( center - y );
		qreal max = (ICONSIZEHEIGHT + ITEMSPACING) / 2.0;
		if ( offset >= max ) {
			g->setY( (ITEMSPACING * 3.0 / 2.0) + ICONSIZEHEIGHT + (i * (ICONSIZEHEIGHT + ITEMSPACING)) );
		}
		else {
			offset = (ICONSIZEHEIGHT + ITEMSPACING) - (offset * (ICONSIZEHEIGHT + ITEMSPACING) / max);
			for ( ; i < dragList.count(); ++i )
				dragList.at( i )->setY( (ITEMSPACING * 3.0 / 2.0) + ICONSIZEHEIGHT + (i * (ICONSIZEHEIGHT + ITEMSPACING)) + offset );
			break;
		}
	}
	it->setY( y );		
}



void Graph::effectReleased( GraphEffectItem *it )
{
	int i, j, index = -1;
	
	QList<QGraphicsItem*> list = items();
	QList<GraphEffectItem*> dragList;
	for ( i = 0; i < list.count(); ++i ) {
		GraphItem *e = (GraphItem*)list.at(i);
		if ( e->isEffect ) {
			for ( j = 0; j < dragList.count(); ++j ) {
				if ( e->y() < dragList.at(j)->y() ) {
					dragList.insert( j, (GraphEffectItem*)e );
					break;
				}
			}
			if ( j == dragList.count() )
				dragList.append( (GraphEffectItem*)e );
		}
	}
	for ( i = 0; i < dragList.count(); ++i ) {
		GraphEffectItem *e = dragList.at(i);
		if ( e == it ) {
			index = i;
			break;
		}
	}
	
	if ( index != -1 ) {
		emit filterReordered( currentClip->getClip(), !isAudio, it->index(), index );
	}
}



void Graph::dragEnterEvent( QGraphicsSceneDragDropEvent *event )
{
	const QMimeData *mimeData = event->mimeData();
	if ( currentClip && mimeData->formats().contains( MIMETYPEEFFECT ) ) {
		dragList.clear();
		QList<QGraphicsItem*> list = items();
		for ( int i = 0; i < list.count(); ++i ) {
			GraphItem *g = (GraphItem*)list.at(i);
			if ( g->isEffect ) {
				int j = 0;
				for ( ; j < dragList.count(); ++j ) {
					if ( g->y() < dragList.at(j)->y() ) {
						dragList.insert( j, (GraphEffectItem*)g );
						break;
					}
				}
				if ( j == dragList.count() )
					dragList.append( (GraphEffectItem*)g );
			}
		}
		event->accept();
	}
	else
		QGraphicsScene::dragEnterEvent( event );
}



void Graph::dragMoveEvent( QGraphicsSceneDragDropEvent *event )
{
	const QMimeData *mimeData = event->mimeData();
	if ( currentClip && mimeData->formats().contains( MIMETYPEEFFECT ) ) {
		qreal y = event->scenePos().y();
		if ( y <= (ITEMSPACING + ICONSIZEHEIGHT) / 2.0 ) {
			QGraphicsScene::dragMoveEvent( event );
			return;
		}
		for ( int i = 0; i < dragList.count(); ++i ) {
			GraphEffectItem *g = dragList.at( i );
			qreal center = (ITEMSPACING * 3.0 / 2.0) + ICONSIZEHEIGHT + (i * (ICONSIZEHEIGHT + ITEMSPACING));
			qreal offset = qAbs( center - y );
			qreal max = (ICONSIZEHEIGHT + ITEMSPACING) / 2.0;
			if ( offset >= max ) {
				g->setY( (ITEMSPACING * 3.0 / 2.0) + ICONSIZEHEIGHT + (i * (ICONSIZEHEIGHT + ITEMSPACING)) );
			}
			else {
				offset = (ICONSIZEHEIGHT + ITEMSPACING) - (offset * (ICONSIZEHEIGHT + ITEMSPACING) / max);
				for ( ; i < dragList.count(); ++i )
					dragList.at( i )->setY( (ITEMSPACING * 3.0 / 2.0) + ICONSIZEHEIGHT + (i * (ICONSIZEHEIGHT + ITEMSPACING)) + offset );
				break;
			}
		}
		event->accept();
	}
	else
		QGraphicsScene::dragMoveEvent( event );
}



void Graph::dragLeaveEvent( QGraphicsSceneDragDropEvent *event )
{
	for ( int i = 0; i < dragList.count(); ++i )
		dragList.at( i )->setY( (ITEMSPACING * 3.0 / 2.0) + ICONSIZEHEIGHT + (i * (ICONSIZEHEIGHT + ITEMSPACING)) );
	QGraphicsScene::dragLeaveEvent( event );
}



void Graph::dropEvent( QGraphicsSceneDragDropEvent *event )
{
	const QMimeData *mimeData = event->mimeData();
	if ( currentClip && mimeData->formats().contains( MIMETYPEEFFECT ) ) {
		QString fx = mimeData->data( MIMETYPEEFFECT ).data();
		qreal y = event->scenePos().y();
		int index = -1;
		for ( int i = 0; i < dragList.count(); ++i ) {
			if ( y <= dragList.at( i )->y() + (ICONSIZEHEIGHT / 2.0) ) {
				index = i;
				break;
			}
		}
		emit filterAdded( currentClip, fx, index );
		event->accept();
	}
	else
		QGraphicsScene::dropEvent( event );
}
