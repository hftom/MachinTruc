#include <QDebug>
#include <QMimeData>

#include "engine/filtercollection.h"
#include "gui/mimetypes.h"
#include "gui/topwindow.h"
#include "timeline/timeline.h"

#define MINTRACKLENGTH 300
#define DEFAULTZOOM 102400.0
#define ZOOMMAX 3276800.0
#define ZOOMMIN 800.0



Timeline::Timeline( TopWindow *parent ) : QGraphicsScene(),
	zoom( DEFAULTZOOM ),
	viewWidth( 0 ),
	moveItem( NULL ),
	selectedItem( NULL ),
	topParent( parent )
{
	setBackgroundBrush( QBrush( QColor(20,20,20) ) );
	
	cursor = new CursorViewItem();
	addItem( cursor );
	cursor->setZValue( ZCURSOR );
	cursor->setPos( 0, 0 );	
}



Timeline::~Timeline()
{
}



void Timeline::viewSizeChanged( const QSize &size )
{
	viewWidth = size.width();
	QTimer::singleShot ( 1, this, SLOT(updateLength()) );
}



void Timeline::wheelEvent( QGraphicsSceneWheelEvent *e )
{
	if ( e->modifiers() & Qt::ControlModifier ) {
		double oldzoom = zoom;
		
		if ( e->delta() > 0 ) {
			if ( zoom > ZOOMMIN )
				zoom /= 2.0;
		}
		else {
			if ( zoom < ZOOMMAX )
				zoom *= 2;
		}

		if ( oldzoom != zoom ) {
			int i, j;
			for ( i = 0; i < tracks.count(); ++i ) {
				TrackViewItem *tv = tracks.at( i );
				QList<QGraphicsItem*> list = tv->childItems();
				for ( j = 0; j < list.count(); ++j ) {
					AbstractViewItem *it = (AbstractViewItem*)list.at( j );
					it->setScale( zoom );
				}
			}
			cursor->setX( (cursor->x() * oldzoom) / zoom );
			updateLength();
			emit centerOn( cursor );
			update();
		}
		
		e->accept();
	}
}



void Timeline::itemSelected( AbstractViewItem *it )
{
	if ( selectedItem )
		selectedItem->setSelected( false );
	if ( it ) {
		it->setSelected (true );
		selectedItem = it;
		if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			ClipViewItem *c = (ClipViewItem*)it;
			emit clipSelected( c->getClip() );
		}
	}
	else {
		selectedItem = NULL;
		emit clipSelected( NULL );
	}
}



void Timeline::trackPressed( QPointF p )
{
	double pts = p.x() * zoom;
	qint64 i = pts / scene->profile.getVideoFrameDuration();
	pts = i;
	emit seekTo( pts * scene->profile.getVideoFrameDuration() );
}



void Timeline::removeEndTransition( ClipViewItem *clip )
{
	QList<QGraphicsItem*> list = items( clip->mapToScene( clip->rect().topRight() ), Qt::IntersectsItemBoundingRect, Qt::AscendingOrder );
	for ( int i = 0; i < list.count(); ++i ) {
		QGraphicsItem *it = list.at( i );
		if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			ClipViewItem *cv = (ClipViewItem*)it;
			if ( cv != clip ) {
				cv->updateTransition( 0 );
				break;
			}
		}
	}
}

void Timeline::updateEndTransition( ClipViewItem *clip )
{
	QList<QGraphicsItem*> list = items( clip->mapToScene( clip->rect().topRight() ), Qt::IntersectsItemBoundingRect, Qt::AscendingOrder );
	for ( int i = 0; i < list.count(); ++i ) {
		QGraphicsItem *it = list.at( i );
		if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			ClipViewItem *cv = (ClipViewItem*)it;
			if ( cv != clip ) {
				cv->updateTransition( clip->getPosition() + clip->getLength() - cv->getPosition() );
				break;
			}
		}
	}
}

void Timeline::removeStartTransition( ClipViewItem *clip )
{
	QList<QGraphicsItem*> list = items( clip->mapToScene( clip->rect().topLeft() ), Qt::IntersectsItemBoundingRect, Qt::AscendingOrder );
	for ( int i = 0; i < list.count(); ++i ) {
		QGraphicsItem *it = list.at( i );
		if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			ClipViewItem *cv = (ClipViewItem*)it;
			if ( cv != clip ) {
				clip->updateTransition( 0 );
				break;
			}
		}
	}
}

void Timeline::updateStartTransition( ClipViewItem *clip )
{
	QList<QGraphicsItem*> list = items( clip->mapToScene( clip->rect().topLeft() ), Qt::IntersectsItemBoundingRect, Qt::AscendingOrder );
	for ( int i = 0; i < list.count(); ++i ) {
		QGraphicsItem *it = list.at( i );
		if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			ClipViewItem *cv = (ClipViewItem*)it;
			if ( cv != clip ) {
				clip->updateTransition( cv->getPosition() + cv->getLength() - clip->getPosition() );
				break;
			}
		}
	}
}



void Timeline::clipItemCanMove( ClipViewItem *clip, QPointF mouse, double clipStartPos, QPointF clipStartMouse, bool unsnap )
{
	double newPos = ( mouse.x() * zoom ) - ( (clipStartMouse.x() * zoom) - clipStartPos );
		
	if ( !unsnap )
		snapMove( clip, newPos, mouse.x(), ( clipStartMouse.x() - (clipStartPos / zoom) ) );
		
	int itemTrack = getTrack( clip->sceneBoundingRect().topLeft() );
	if ( newPos < 0 )
		newPos = 0;
	int newTrack = getTrack( mouse );
	if ( newTrack < 0 )
		newTrack = itemTrack;
	if ( scene->canMove( clip->getClip(), clip->getClip()->length(), newPos, newTrack ) ) {
		removeStartTransition( clip );
		removeEndTransition( clip );
		clip->setParentItem( tracks.at( newTrack ) );
		clip->setPosition( newPos );
		updateStartTransition( clip );
		updateEndTransition( clip );
	}
}



void Timeline::clipItemMoved( ClipViewItem *clip, QPointF clipStartMouse )
{
	scene->move( clip->getClip(), getTrack( clipStartMouse ), clip->getPosition(), getTrack( clip->sceneBoundingRect().topLeft() ) );
	QTimer::singleShot ( 1, this, SIGNAL(updateFrame()) );
	QTimer::singleShot ( 1, this, SLOT(updateLength()) );
}



void Timeline::clipItemCanResize( ClipViewItem *clip, int way, QPointF mouse, double clipStartPos, double clipStartLen, QPointF clipStartMouse, bool unsnap )
{
	int track = getTrack( clip->sceneBoundingRect().topLeft() );
	
	if ( way == 2 ) {
		double newLength = clipStartLen + ( mouse.x() * zoom ) - ( clipStartMouse.x() * zoom);
		if ( !unsnap )
			snapResize( clip, way, newLength, mouse.x(), ( clipStartMouse.x() - ((clip->getPosition() + clipStartLen) / zoom) ) );
		if ( scene->canResize( clip->getClip(), newLength, track ) )
			clip->setLength( newLength );
	}
	else {
		double newPos = ( mouse.x() * zoom ) - ( (clipStartMouse.x() * zoom) - clipStartPos );
		if ( !unsnap )
			snapResize( clip, way, newPos, mouse.x(), ( clipStartMouse.x() - ( clipStartPos / zoom ) ) );
		if ( newPos < 0 )
			newPos = 0;
		double endPos = clipStartPos + clipStartLen;
		if ( scene->canResizeStart( clip->getClip(), newPos, endPos, track ) ) {
			clip->setGeometry( newPos, endPos - newPos );
		}
	}
}



void Timeline::clipItemResized( ClipViewItem *clip, int way )
{
	if ( way == 2 )
		scene->resize( clip->getClip(), clip->getLength(), getTrack( clip->sceneBoundingRect().topLeft() ) );
	else
		scene->resizeStart( clip->getClip(), clip->getPosition(), clip->getLength(), getTrack( clip->sceneBoundingRect().topLeft() ) );
	QTimer::singleShot ( 1, this, SIGNAL(updateFrame()) );
	QTimer::singleShot ( 1, this, SLOT(updateLength()) );
}



void Timeline::snapResize( ClipViewItem *item, int way, double &poslen, double mouseX, double itemScenePos )
{
	QRectF src = item->mapRectToScene( item->rect() );
	double delta = fabs( itemScenePos - ( mouseX - ( way == 2 ? src.right() : src.left() ) ) );
	if ( delta > SNAPWIDTH )
		return;
	QList<QGraphicsItem*> snapItems = items( src.left() - SNAPWIDTH, 0, src.right() + SNAPWIDTH, cursor->rect().height(), Qt::IntersectsItemBoundingRect, Qt::AscendingOrder );
	int i;
	for ( i = 0; i < snapItems.count(); ++i ) {
		QGraphicsItem *it = snapItems.at( i );
		if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			ClipViewItem *cv = (ClipViewItem*)it;
			if ( cv != item ) {
				QRectF dst = cv->mapRectToScene( cv->rect() );
				if ( way == 2 ) {
					if ( (dst.left() > src.right() - SNAPWIDTH) && (dst.left() < src.right() + SNAPWIDTH) ) {
						poslen = cv->getPosition() - item->getPosition() + (scene->profile.getVideoFrameDuration() / 4.0);
						return;
					}
					else if ( (dst.right() > src.right() - SNAPWIDTH) && (dst.right() < src.right() + SNAPWIDTH) ) {
						poslen = cv->getPosition() + cv->getLength() - item->getPosition() + (scene->profile.getVideoFrameDuration() / 4.0);
						return;
					}				
				}
				else {
					if ( (dst.left() > src.left() - SNAPWIDTH) && (dst.left() < src.left() + SNAPWIDTH) ) {
						poslen = cv->getPosition() + (scene->profile.getVideoFrameDuration() / 4.0);
						return;
					}
					else if ( (dst.right() > src.left() - SNAPWIDTH) && (dst.right() < src.left() + SNAPWIDTH) ) {
						poslen = cv->getPosition() + cv->getLength() + (scene->profile.getVideoFrameDuration() / 4.0);
						return;
					}
				}
			}
		}
		else if ( it->data( DATAITEMTYPE ).toInt() == TYPECURSOR ) {
			QRectF dst = cursor->mapRectToScene( cursor->rect() );
			if ( way == 2 ) {
				if ( (dst.left() > src.right() - SNAPWIDTH) && (dst.left() < src.right() + SNAPWIDTH) ) {
					poslen = (dst.left() * zoom) - item->getPosition() + (scene->profile.getVideoFrameDuration() / 4.0);
					return;
				}				
			}
			else {
				if ( (dst.left() > src.left() - SNAPWIDTH) && (dst.left() < src.left() + SNAPWIDTH) ) {
					poslen = (dst.left() * zoom) + (scene->profile.getVideoFrameDuration() / 4.0);
					return;
				}
			}
		}
	}
}



void Timeline::snapMove( ClipViewItem *item, double &pos, double mouseX, double itemScenePos )
{
	QRectF src = item->mapRectToScene( item->rect() );
	double delta = fabs( itemScenePos - ( mouseX - src.left() ) );
	if ( delta > SNAPWIDTH )
		return;
	QList<QGraphicsItem*> snapItems = items( src.left() - SNAPWIDTH, 0, src.right() + SNAPWIDTH, cursor->rect().height(), Qt::IntersectsItemBoundingRect, Qt::AscendingOrder );
	int i;
	for ( i = 0; i < snapItems.count(); ++i ) {
		QGraphicsItem *it = snapItems.at( i );
		if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			ClipViewItem *cv = (ClipViewItem*)it;
			if ( cv != item ) {
				QRectF dst = cv->mapRectToScene( cv->rect() );
				if ( (dst.left() > src.left() - SNAPWIDTH) && (dst.left() < src.left() + SNAPWIDTH) ) {
					pos = cv->getPosition() + (scene->profile.getVideoFrameDuration() / 4.0);
					return;
				}
				else if ( (dst.right() > src.left() - SNAPWIDTH) && (dst.right() < src.left() + SNAPWIDTH) ) {
					pos = cv->getPosition() + cv->getLength() + (scene->profile.getVideoFrameDuration() / 4.0);
					return;
				}
				else if ( (dst.left() > src.right() - SNAPWIDTH) && (dst.left() < src.right() + SNAPWIDTH) ) {
					pos = cv->getPosition() - item->getLength() + (scene->profile.getVideoFrameDuration() / 4.0);
					return;
				}
				else if ( (dst.right() > src.right() - SNAPWIDTH) && (dst.right() < src.right() + SNAPWIDTH) ) {
					pos = cv->getPosition() + cv->getLength() - item->getLength() + (scene->profile.getVideoFrameDuration() / 4.0);
					return;
				}				
			}
		}
		else if ( it->data( DATAITEMTYPE ).toInt() == TYPECURSOR ) {
			QRectF dst = cursor->mapRectToScene( cursor->rect() );
			if ( (dst.left() > src.left() - SNAPWIDTH) && (dst.left() < src.left() + SNAPWIDTH) ) {
				pos = (dst.left() * zoom) + (scene->profile.getVideoFrameDuration() / 4.0);
				return;
			}
			else if ( (dst.left() > src.right() - SNAPWIDTH) && (dst.left() < src.right() + SNAPWIDTH) ) {
				pos = (dst.left() * zoom) - item->getLength() + (scene->profile.getVideoFrameDuration() / 4.0);
				return;
			}
		}
	}
}



int Timeline::getTrack( const QPointF &p )
{
	int i;
	
	for ( i = 0; i < tracks.count(); ++i ) {
		if ( tracks.at( i )->sceneBoundingRect().contains( QPointF( 0, p.y() ) ) )
			return i;
	}
	
	return -1;
}



void Timeline::setCursorPos( double pts )
{
	double d = scene->profile.getVideoFrameDuration();
	qint64 i = ( pts + ( d / 2.0 ) ) / d;
	pts = i * d;
	cursor->setX( pts / zoom );
	emit ensureVisible( cursor );
}



void Timeline::addTrack( int index )
{
	TrackViewItem *tv = new TrackViewItem();
	tracks.insert( index, tv );
	addItem( tv );
	int i;
	for ( i = 0; i < tracks.count(); ++i )
		tracks.at( tracks.count() - 1 - i )->setPos( 0, (TRACKVIEWITEMHEIGHT + 1) * i );
	
	cursor->setHeight( tracks.count() * (TRACKVIEWITEMHEIGHT + 1) );
}



void Timeline::updateLength()
{
	int i, j, maxlen=0;
	
	for ( i = 0; i < tracks.count(); ++i ) {
		TrackViewItem *tv = tracks.at( i );
		QList<QGraphicsItem*> list = tv->childItems();
		for ( j = 0; j < list.count(); ++j ) {
			AbstractViewItem *it = (AbstractViewItem*)list.at( j );
			if ( it->pos().x() + it->rect().width() > maxlen )
				maxlen = it->pos().x() + it->rect().width();
		}
	}
	
	maxlen += MINTRACKLENGTH;
	
	if ( maxlen < viewWidth )
		maxlen = viewWidth;

	for ( i = 0; i < tracks.count(); ++i ) {
		TrackViewItem *tv = tracks.at( i );
		QRectF r = tv->rect();
		r.setWidth( maxlen );
		tracks.at( i )->setRect( r );
	}
	
	setSceneRect( 0, 0, maxlen, tracks.count() * TRACKVIEWITEMHEIGHT );
}



void Timeline::setScene( Scene *s )
{
	int i, j;
	
	for ( i = 0; i < tracks.count(); ++i ) {
		QGraphicsItem *track = tracks.at( i );
		QList<QGraphicsItem*> list = track->childItems();
		while ( list.count() ) {
			QGraphicsItem *it = list.takeFirst();
			removeItem( it );
			delete it;
		}
	}
	while ( tracks.count() ) {
		QGraphicsItem *it = tracks.takeFirst();
		removeItem( it );
		delete it;
	}

	for ( i = 0; i < s->tracks.count(); ++i ) {
		addTrack( i );
		Track *t = s->tracks[i];
		for ( j = 0; j < t->clipCount(); ++j ) {
			Clip *c = t->clipAt( j );
			QGraphicsItem *it = new ClipViewItem( c, zoom );
			addItem( it );
			it->setParentItem( tracks.at( i ) );
		}
	}
	
	scene = s;

	QTimer::singleShot ( 1, this, SLOT(updateLength()) );
}



void Timeline::deleteClip()
{
	if ( selectedItem ) {
		if ( scene->removeClip( ((ClipViewItem*)selectedItem)->getClip() ) ) {
			removeItem( selectedItem );
			delete selectedItem;
			selectedItem = NULL;
			itemSelected( NULL );
			emit updateFrame();
			QTimer::singleShot ( 1, this, SLOT(updateLength()) );
		}
	}
}



void Timeline::splitCurrentClip()
{
	if ( selectedItem && selectedItem->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
		ClipViewItem *cv = (ClipViewItem*)selectedItem;
		double cursor_pts = cursor->mapRectToScene( cursor->rect() ).left() * zoom ;
		int t = getTrack( cv->sceneBoundingRect().topLeft() );
		Clip *current_clip = cv->getClip();
		Clip *c = scene->sceneSplitClip( current_clip, t, cursor_pts );
		if ( c ) {
			QGraphicsItem *it = new ClipViewItem( c, zoom );
			addItem( it );
			it->setParentItem( tracks.at( t ) );
			cv->setLength( current_clip->length() );
			itemSelected( (ClipViewItem*)it );
			emit updateFrame();
		}
	}
}



void Timeline::addFilter( ClipViewItem *clip, QString fx )
{
	int i;
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	for ( i = 0; i < fc->videoFilters.count(); ++i ) {
		if ( fc->videoFilters[ i ].identifier == fx ) {
			QSharedPointer<Filter> f = fc->videoFilters[ i ].create();
			f->setPosition( clip->getClip()->position() );
			f->setLength( clip->getClip()->length() );
			clip->getClip()->videoFilters.append( f.staticCast<GLFilter>() );
			itemSelected( clip );
			emit updateFrame();
			break;
		}
	}
}



void Timeline::filterDeleted( Clip *c, QSharedPointer<Filter> f )
{
	if ( !c->videoFilters.remove( f.staticCast<GLFilter>() ) )
		c->audioFilters.remove( f.staticCast<AudioFilter>() );

	emit updateFrame();
}



void Timeline::dragEnterEvent( QGraphicsSceneDragDropEvent *event )
{
	const QMimeData *mimeData = event->mimeData();
	QString t;
	if ( mimeData->formats().contains( MIMETYPECUT ) )
		t = mimeData->data( MIMETYPECUT ).data();
	else if ( mimeData->formats().contains( MIMETYPESOURCE ) )
		t = mimeData->data( MIMETYPESOURCE ).data();
	int i = t.indexOf( " " );
	if ( i != -1 ) {
		bool ok = false;
		int index = t.left( i ).toInt( &ok );
		if ( ok ) {
			double start, len;
			QString filename = t.remove( 0, i ).trimmed();
			Source* source = topParent->getDroppedCut( index, mimeData->formats().at( 0 ), filename, start, len );
			if ( source ) {
				if ( !droppedCut.clip ) {
					double newPos = event->scenePos().x() * zoom;
					if ( newPos < 0 )
						newPos = 0;
					
					droppedCut.clip = scene->createClip( source, newPos, start, len );
					droppedCut.clipItem = new ClipViewItem( droppedCut.clip, zoom );
					droppedCut.enterPos = event->scenePos().x() - ( droppedCut.clipItem->getPosition() / zoom );
					snapMove( droppedCut.clipItem, newPos, event->scenePos().x(), droppedCut.enterPos );
					if ( newPos < 0 )
						newPos = 0;

					int newTrack = getTrack( event->scenePos() );
					if ( newTrack < 0 )
						return;
					if ( scene->canMove( droppedCut.clip, droppedCut.clip->length(), newPos, newTrack ) ) {
						addItem( droppedCut.clipItem );
						itemSelected( droppedCut.clipItem );
						droppedCut.shown = true;
						droppedCut.clipItem->setParentItem( tracks.at( newTrack ) );
						droppedCut.clipItem->setPosition( newPos );
						droppedCut.clip->setPosition( newPos );
					}
				}
				event->accept();
				return;
			}
		}
	}
	QGraphicsScene::dragEnterEvent( event );
}



void Timeline::dragMoveEvent( QGraphicsSceneDragDropEvent *event )
{
	if ( droppedCut.clip ) {
		double newPos = event->scenePos().x() * zoom;
		
		snapMove( droppedCut.clipItem, newPos, event->scenePos().x(), droppedCut.enterPos );
		
		if ( newPos < 0 )
			newPos = 0;

		int newTrack = getTrack( event->scenePos() );
		int itemTrack = getTrack( droppedCut.clipItem->sceneBoundingRect().topLeft() );
		if ( newTrack < 0 ) {
			if ( !droppedCut.shown )
				return;
			newTrack = itemTrack;
		}
		if ( scene->canMove( droppedCut.clip, droppedCut.clip->length(), newPos, newTrack ) ) {
			if ( !droppedCut.shown ) {
				addItem( droppedCut.clipItem );
				droppedCut.shown = true;
				itemSelected( droppedCut.clipItem );
			}
			droppedCut.clipItem->setParentItem( tracks.at( newTrack ) );
			droppedCut.clipItem->setPosition( newPos );
			droppedCut.clip->setPosition( newPos );
		}
	}
	else
		QGraphicsScene::dragMoveEvent( event );
}



void Timeline::dragLeaveEvent( QGraphicsSceneDragDropEvent *event )
{
	if ( droppedCut.clip ) {
		if ( droppedCut.shown ) {
			selectedItem = NULL;
			itemSelected( NULL );
			removeItem( droppedCut.clipItem );
		}
		droppedCut.destroy();
	}
	else
		QGraphicsScene::dragLeaveEvent( event );
}



void Timeline::dropEvent( QGraphicsSceneDragDropEvent *event )
{
	if ( droppedCut.clip ) {
		if ( droppedCut.shown ) {
			scene->addClip( droppedCut.clip, getTrack( droppedCut.clipItem->sceneBoundingRect().topLeft() ) );
			emit updateFrame();
			QTimer::singleShot ( 1, this, SLOT(updateLength()) );
			droppedCut.reset();
		}
		else {
			droppedCut.destroy();
		}
	}
	else
		QGraphicsScene::dropEvent( event );
}
