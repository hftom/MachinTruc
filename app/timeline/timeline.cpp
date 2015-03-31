#include <QDebug>
#include <QMimeData>
#include <QInputDialog>
#include <QMessageBox>

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
	selectedItem( NULL ),
	effectItem( NULL ),
	scene( NULL ),
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
	if ( effectItem ) {
		removeItem( effectItem );
		delete effectItem;
		effectItem = NULL;
	}

	if ( selectedItem )
		selectedItem->setSelected( false );
	if ( it ) {
		it->setSelected (true );
		selectedItem = it;
		if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			emit clipSelected( (ClipViewItem*)it );
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
	qint64 i = pts / scene->getProfile().getVideoFrameDuration();
	pts = i;
	emit seekTo( pts * scene->getProfile().getVideoFrameDuration() );
}



void Timeline::playheadMoved( double p )
{
	double pts = p * zoom;
	qint64 i = pts / scene->getProfile().getVideoFrameDuration();
	pts = i;
	emit seekTo( pts * scene->getProfile().getVideoFrameDuration() );
}


#define ADDABOVE 100
#define ADDBELOW 101
#define RMTRACK 102
void Timeline::trackPressedRightBtn( TrackViewItem *t, QPoint p )
{
	QMenu menu;
	QAction *action = menu.addAction( tr("Add track above") );
	action->setData( ADDABOVE );
	action = menu.addAction( tr("Add track below") );
	action->setData( ADDBELOW );
	action = menu.addAction( tr("Delete track") );
	action->setData( RMTRACK );
	action = menu.exec( p );
	
	if ( !action )
		return;
	
	for ( int i = 0; i < tracks.count(); ++i ) {
		if ( tracks[i] == t ) {
			int what = action->data().toInt();
			switch ( what ) {
				case ADDABOVE: emit trackRequest( false, i + 1 ); break;
				case ADDBELOW: emit trackRequest( false, i ); break;
				case RMTRACK: {
					if ( tracks.count() < 2 )
						return;
					emit trackRequest( true, i ); break;
				}
			}
			break;
		}
	}
}



void Timeline::trackRemoved( int index )
{
	QGraphicsItem *it = tracks.takeAt( index );
	removeItem( it );
	delete it;
	
	for ( int i = 0; i < tracks.count(); ++i )
		tracks.at( tracks.count() - 1 - i )->setPos( 0, (TRACKVIEWITEMHEIGHT + 1) * i );

	cursor->setHeight( tracks.count() * (TRACKVIEWITEMHEIGHT + 1) );
	QTimer::singleShot ( 1, this, SLOT(updateLength()) );
}



void Timeline::trackAdded( int index )
{
	addTrack( index );
}



void Timeline::clipDoubleClicked()
{
	emit showEffects();
}



void Timeline::showEffect( bool isVideo, int index )
{
	if ( effectItem ) {
		removeItem( effectItem );
		delete effectItem;
		effectItem = NULL;
	}
	if ( index == -1 )
		return;
	
	ClipViewItem *cv = (ClipViewItem*)selectedItem;
	QSharedPointer<Filter> f;
	if ( isVideo )
		f = cv->getClip()->videoFilters.at( index );
	else
		f = cv->getClip()->audioFilters.at( index );
	effectItem = new ClipEffectViewItem( cv->getClip(), f->getFilterName(), isVideo, index, zoom );
	effectItem->setParentItem( tracks.at( getTrack( cv->sceneBoundingRect().topLeft() ) ) );
}



void Timeline::transitionSelected( TransitionViewItem *it )
{
	QList<QGraphicsItem*> list = items( it->mapToScene( it->rect().topLeft() ), Qt::IntersectsItemBoundingRect, Qt::AscendingOrder );
	for ( int i = 0; i < list.count(); ++i ) {
		QGraphicsItem *item = list.at( i );
		if ( item->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			ClipViewItem *cv = (ClipViewItem*)item;
			if ( cv->getTransition() == it )  {
				itemSelected( cv );
				emit showTransition();
				break;
			}
		}
	}
}



void Timeline::clipRightClick( ClipViewItem *cv )
{
	QMenu menu;
	menu.addAction( tr("Clip speed") );
	QAction *action = menu.exec( QCursor::pos() );
	if ( !action )
		return;

	double oldSpeed = cv->getClip()->getSpeed();
	double speed = QInputDialog::getDouble( topParent, tr("Clip speed"), tr("Set a negative value to reverse."), oldSpeed, -10.0, 10.0, 2 );
	if ( speed == 0 )
		speed = 0.01;
	if ( speed != oldSpeed ) {
		double newLength = (1.0 / qAbs(speed)) * cv->getClip()->length() / (1.0 / qAbs(oldSpeed));
		int track = getTrack( cv->sceneBoundingRect().topLeft() );
		// set new speed now, since scene->canResize needs it
		// and restore oldSpeed if we can't resize
		// We set a positive speed so that scene->canResize
		// does not change clip->start
		cv->getClip()->setSpeed( qAbs(speed) );
		if ( scene->canResize( cv->getClip(), newLength, track ) ) {
			updateTransitions( cv, true );
			cv->setLength( newLength );
			updateTransitions( cv, false );
			scene->resize( cv->getClip(), newLength, track );
			// set the real speed now
			cv->getClip()->setSpeed( speed );
			clipThumbRequest( cv, true );
			clipThumbRequest( cv, false );
			// force scene update
			scene->update = true;
			QTimer::singleShot ( 1, this, SIGNAL(updateFrame()) );
			QTimer::singleShot ( 1, this, SLOT(updateLength()) );
		}
		else {
			QMessageBox::warning( topParent, tr("Warning"), tr("There is not enougth room to resize this clip. Move next clips and try again.") );
			cv->getClip()->setSpeed( oldSpeed );
		}
	}
}



void Timeline::effectCanMove( QPointF mouse, double clipStartPos, QPointF clipStartMouse, bool unsnap )
{
	double newPos = ( mouse.x() * zoom ) - ( (clipStartMouse.x() * zoom) - clipStartPos );
		
	if ( !unsnap )
		snapMove( effectItem, newPos, mouse.x(), ( clipStartMouse.x() - (clipStartPos / zoom) ), false );

	if ( scene->effectCanMove( effectItem->getClip(), newPos, effectItem->isVideoEffect(), effectItem->getIndex() ) ) {
		effectItem->setPosition( newPos );
	}
}



void Timeline::effectMoved( QPointF clipMouseStart )
{
	scene->effectMove( effectItem->getClip(), effectItem->getPosition(), effectItem->isVideoEffect(), effectItem->getIndex() );
	QTimer::singleShot ( 1, this, SIGNAL(updateFrame()) );
}



void Timeline::effectCanResize( int way, QPointF mouse, double clipStartPos, double clipStartLen, QPointF clipStartMouse, bool unsnap )
{
	if ( way == 2 ) {
		double newLength = clipStartLen + ( mouse.x() * zoom ) - ( clipStartMouse.x() * zoom);
		if ( !unsnap )
			snapResize( effectItem, way, newLength, mouse.x(), ( clipStartMouse.x() - ((effectItem->getPosition() + clipStartLen) / zoom) ) );
		if ( scene->effectCanResize( effectItem->getClip(), newLength, effectItem->isVideoEffect(), effectItem->getIndex() ) ) {
			effectItem->setLength( newLength );
		}
	}
	else {
		double newPos = ( mouse.x() * zoom ) - ( (clipStartMouse.x() * zoom) - clipStartPos );
		if ( !unsnap )
			snapResize( effectItem, way, newPos, mouse.x(), ( clipStartMouse.x() - ( clipStartPos / zoom ) ) );
		if ( newPos < 0 )
			newPos = 0;
		double endPos = clipStartPos + clipStartLen;
		if ( scene->effectCanResizeStart( effectItem->getClip(), newPos, endPos, effectItem->isVideoEffect(), effectItem->getIndex() ) ) {
			effectItem->setGeometry( newPos, endPos - newPos );
		}
	}
}



void Timeline::effectResized( int way )
{
	if ( way == 2 )
		scene->effectResize( effectItem->getClip(), effectItem->getLength(), effectItem->isVideoEffect(), effectItem->getIndex() );
	else
		scene->effectResizeStart( effectItem->getClip(), effectItem->getPosition(), effectItem->getLength(), effectItem->isVideoEffect(), effectItem->getIndex() );
	QTimer::singleShot ( 1, this, SIGNAL(updateFrame()) );
}



void Timeline::updateTransitions( ClipViewItem *clip, bool remove )
{
	// begin
	ClipViewItem *begin = NULL;
	QList<QGraphicsItem*> list = items( clip->mapToScene( clip->rect().topLeft() ), Qt::IntersectsItemBoundingRect, Qt::AscendingOrder );
	for ( int i = 0; i < list.count(); ++i ) {
		QGraphicsItem *it = list.at( i );
		if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			ClipViewItem *cv = (ClipViewItem*)it;
			if ( cv != clip ) {
				clip->updateTransition( remove ? 0 : cv->getPosition() + cv->getLength() - clip->getPosition() );
				begin = cv;
				break;
			}
		}
	}
	// end
	QPointF p = clip->mapToScene( clip->rect().topRight() );
	p.rx() -= scene->getProfile().getVideoFrameDuration() / 2.0 / zoom;
	list = items( p, Qt::IntersectsItemBoundingRect, Qt::AscendingOrder );
	for ( int i = 0; i < list.count(); ++i ) {
		QGraphicsItem *it = list.at( i );
		if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			ClipViewItem *cv = (ClipViewItem*)it;
			if ( cv != clip && cv != begin ) {
				cv->updateTransition( remove ? 0 : clip->getPosition() + clip->getLength() - cv->getPosition() );
				break;
			}
		}
	}
}



void Timeline::clipItemCanMove( ClipViewItem *clip, QPointF mouse, double clipStartPos, QPointF clipStartMouse, bool unsnap, bool multiMove )
{
	double newPos = ( mouse.x() * zoom ) - ( (clipStartMouse.x() * zoom) - clipStartPos );
		
	if ( !unsnap )
		snapMove( clip, newPos, mouse.x(), ( clipStartMouse.x() - (clipStartPos / zoom) ), multiMove );
		
	int itemTrack = getTrack( clip->sceneBoundingRect().topLeft() );
	if ( newPos < 0 )
		newPos = 0;
	int newTrack = getTrack( mouse );
	if ( newTrack < 0 )
		newTrack = itemTrack;
	if ( multiMove ) {
		if ( scene->canMoveMulti( clip->getClip(), clip->getClip()->length(), newPos, itemTrack ) ) {
			double start = clip->getPosition();
			double delta = newPos - start;
			updateTransitions( clip, true );
			QList<QGraphicsItem*> list = tracks.at( itemTrack )->childItems();
			for ( int i = 0; i < list.count(); ++i ) {
				QGraphicsItem *it = list.at( i );
				if ( it->data( DATAITEMTYPE ).toInt() >= TYPECLIP ) {
					AbstractViewItem *av = (AbstractViewItem*)it;
					if ( av->getPosition() < start )
						continue;
					av->moveDelta( delta );
				}
			}
			updateTransitions( clip, false );
		}
	}
	else {
		if ( scene->canMove( clip->getClip(), clip->getClip()->length(), newPos, newTrack ) ) {
			updateTransitions( clip, true );
			clip->setParentItem( tracks.at( newTrack ) );
			clip->setPosition( newPos );
			updateTransitions( clip, false );
		}
	}
}



void Timeline::clipItemMoved( ClipViewItem *clip, QPointF clipStartMouse, bool multiMove )
{
	if ( multiMove ) {
		scene->moveMulti( clip->getClip(), getTrack( clip->sceneBoundingRect().topLeft() ), clip->getPosition() );
	}
	else {
		scene->move( clip->getClip(), getTrack( clipStartMouse ), clip->getPosition(), getTrack( clip->sceneBoundingRect().topLeft() ) );
	}
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
		if ( scene->canResize( clip->getClip(), newLength, track ) ) {
			updateTransitions( clip, true );
			clip->setLength( newLength );
			updateTransitions( clip, false );
		}
	}
	else {
		double newPos = ( mouse.x() * zoom ) - ( (clipStartMouse.x() * zoom) - clipStartPos );
		if ( !unsnap )
			snapResize( clip, way, newPos, mouse.x(), ( clipStartMouse.x() - ( clipStartPos / zoom ) ) );
		if ( newPos < 0 )
			newPos = 0;
		double endPos = clipStartPos + clipStartLen;
		if ( scene->canResizeStart( clip->getClip(), newPos, endPos, track ) ) {
			updateTransitions( clip, true );
			clip->setGeometry( newPos, endPos - newPos );
			updateTransitions( clip, false );
		}
	}
}



void Timeline::clipItemResized( ClipViewItem *clip, int way )
{
	if ( way == 2 )
		scene->resize( clip->getClip(), clip->getLength(), getTrack( clip->sceneBoundingRect().topLeft() ) );
	else
		scene->resizeStart( clip->getClip(), clip->getPosition(), clip->getLength(), getTrack( clip->sceneBoundingRect().topLeft() ) );
	clipThumbRequest( clip, way != 2 );
	QTimer::singleShot ( 1, this, SIGNAL(updateFrame()) );
	QTimer::singleShot ( 1, this, SLOT(updateLength()) );
}



void Timeline::snapResize( AbstractViewItem *item, int way, double &poslen, double mouseX, double itemScenePos )
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
						poslen = cv->getPosition() - item->getPosition() + (scene->getProfile().getVideoFrameDuration() / 4.0);
						return;
					}
					else if ( (dst.right() > src.right() - SNAPWIDTH) && (dst.right() < src.right() + SNAPWIDTH) ) {
						poslen = cv->getPosition() + cv->getLength() - item->getPosition() + (scene->getProfile().getVideoFrameDuration() / 4.0);
						return;
					}				
				}
				else {
					if ( (dst.left() > src.left() - SNAPWIDTH) && (dst.left() < src.left() + SNAPWIDTH) ) {
						poslen = cv->getPosition() + (scene->getProfile().getVideoFrameDuration() / 4.0);
						return;
					}
					else if ( (dst.right() > src.left() - SNAPWIDTH) && (dst.right() < src.left() + SNAPWIDTH) ) {
						poslen = cv->getPosition() + cv->getLength() + (scene->getProfile().getVideoFrameDuration() / 4.0);
						return;
					}
				}
			}
		}
		else if ( it->data( DATAITEMTYPE ).toInt() == TYPECURSOR ) {
			QRectF dst = cursor->mapRectToScene( cursor->rect() );
			if ( way == 2 ) {
				if ( (dst.left() > src.right() - SNAPWIDTH) && (dst.left() < src.right() + SNAPWIDTH) ) {
					poslen = (dst.left() * zoom) - item->getPosition() + (scene->getProfile().getVideoFrameDuration() / 4.0);
					return;
				}				
			}
			else {
				if ( (dst.left() > src.left() - SNAPWIDTH) && (dst.left() < src.left() + SNAPWIDTH) ) {
					poslen = (dst.left() * zoom) + (scene->getProfile().getVideoFrameDuration() / 4.0);
					return;
				}
			}
		}
	}
}



void Timeline::snapMove( AbstractViewItem *item, double &pos, double mouseX, double itemScenePos, bool limit )
{
	QRectF src = item->mapRectToScene( item->rect() );
	double delta = fabs( itemScenePos - ( mouseX - src.left() ) );
	if ( delta > SNAPWIDTH )
		return;
	QList<QGraphicsItem*> snapItems = items( src.left() - SNAPWIDTH, 0, src.right() + SNAPWIDTH, cursor->rect().height(), Qt::IntersectsItemBoundingRect, Qt::AscendingOrder );
	int i;
	double posLimit = item->getPosition() + item->getLength();
	for ( i = 0; i < snapItems.count(); ++i ) {
		QGraphicsItem *it = snapItems.at( i );
		if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			ClipViewItem *cv = (ClipViewItem*)it;
			if ( limit && cv->getPosition() >= posLimit )
				continue;
			if ( cv != item ) {
				QRectF dst = cv->mapRectToScene( cv->rect() );
				if ( (dst.left() > src.left() - SNAPWIDTH) && (dst.left() < src.left() + SNAPWIDTH) ) {
					pos = cv->getPosition() + (scene->getProfile().getVideoFrameDuration() / 4.0);
					return;
				}
				else if ( (dst.right() > src.left() - SNAPWIDTH) && (dst.right() < src.left() + SNAPWIDTH) ) {
					pos = cv->getPosition() + cv->getLength() + (scene->getProfile().getVideoFrameDuration() / 4.0);
					return;
				}
				else if ( (dst.left() > src.right() - SNAPWIDTH) && (dst.left() < src.right() + SNAPWIDTH) ) {
					pos = cv->getPosition() - item->getLength() + (scene->getProfile().getVideoFrameDuration() / 4.0);
					return;
				}
				else if ( (dst.right() > src.right() - SNAPWIDTH) && (dst.right() < src.right() + SNAPWIDTH) ) {
					pos = cv->getPosition() + cv->getLength() - item->getLength() + (scene->getProfile().getVideoFrameDuration() / 4.0);
					return;
				}				
			}
		}
		else if ( it->data( DATAITEMTYPE ).toInt() == TYPECURSOR ) {
			QRectF dst = cursor->mapRectToScene( cursor->rect() );
			if ( (dst.left() > src.left() - SNAPWIDTH) && (dst.left() < src.left() + SNAPWIDTH) ) {
				pos = (dst.left() * zoom) + (scene->getProfile().getVideoFrameDuration() / 4.0);
				return;
			}
			else if ( (dst.left() > src.right() - SNAPWIDTH) && (dst.left() < src.right() + SNAPWIDTH) ) {
				pos = (dst.left() * zoom) - item->getLength() + (scene->getProfile().getVideoFrameDuration() / 4.0);
				return;
			}
		}
	}
}



int Timeline::getTrack( const QPointF &p )
{
	for ( int i = 0; i < tracks.count(); ++i ) {
		if ( tracks.at( i )->sceneBoundingRect().contains( QPointF( 0, p.y() ) ) )
			return i;
	}
	
	return -1;
}



void Timeline::setCursorPos( double pts )
{
	double d = scene->getProfile().getVideoFrameDuration();
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
	QTimer::singleShot ( 1, this, SLOT(updateLength()) );
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

	scene = s;
	selectedItem = NULL;
	effectItem = NULL;

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
			ClipViewItem *it = new ClipViewItem( c, zoom );
			it->setParentItem( tracks.at( i ) );
			Transition *trans = c->getTransition();
			if ( trans )
				it->updateTransition( trans->length() );	
			clipThumbRequest( it, true );
			clipThumbRequest( it, false );
		}
	}
	
	setCursorPos( 0 );
	itemSelected( NULL );

	QTimer::singleShot ( 1, this, SLOT(updateLength()) );
}



void Timeline::deleteClip()
{
	if ( selectedItem ) {
		if ( scene->removeClip( ((ClipViewItem*)selectedItem)->getClip() ) ) {
			updateTransitions( (ClipViewItem*)selectedItem, true );
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
			it->setParentItem( tracks.at( t ) );
			cv->setLength( current_clip->length() );
			itemSelected( (ClipViewItem*)it );
			clipThumbRequest( cv, false );
			clipThumbRequest( (ClipViewItem*)it, true );
			clipThumbRequest( (ClipViewItem*)it, false );
			emit updateFrame();
		}
	}
}



void Timeline::addFilter( ClipViewItem *clip, QString fx, int index )
{
	int i;
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	Clip *c = clip->getClip();
	QSharedPointer<Filter> f;
	for ( i = 0; i < fc->videoFilters.count(); ++i ) {
		if ( fc->videoFilters[ i ].identifier == fx ) {
			f = fc->videoFilters[ i ].create();
			if ( index == -1 )
				c->videoFilters.append( f.staticCast<GLFilter>() );
			else
				c->videoFilters.insert( index, f.staticCast<GLFilter>() );
			break;
		}
	}
	if ( f.isNull() ) {
		for ( i = 0; i < fc->audioFilters.count(); ++i ) {
			if ( fc->audioFilters[ i ].identifier == fx ) {
				f = fc->audioFilters[ i ].create();
				if ( index == -1 )
					c->audioFilters.append( f.staticCast<AudioFilter>() );
				else
					c->audioFilters.insert( index, f.staticCast<AudioFilter>() );
				break;
			}
		}
	}
	
	if ( f.isNull() )
		return;
	
	f->setPosition( c->position() );
	if ( f->getLength() > c->length() )
		f->setLength( c->length() );
	if ( f->getSnap() == Filter::SNAPEND )
		f->setPositionOffset( c->length() - f->getLength() );
	else if ( f->getSnap() == Filter::SNAPSTART )
		f->setPositionOffset( 0 );
	else
		f->setLength( c->length() );
			
	itemSelected( clip );
	emit updateFrame();
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
						itemSelected( droppedCut.clipItem );
						droppedCut.shown = true;
						droppedCut.clipItem->setParentItem( tracks.at( newTrack ) );
						droppedCut.clipItem->setPosition( newPos );
						droppedCut.clip->setPosition( newPos );
						updateTransitions( droppedCut.clipItem, false );
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
				droppedCut.shown = true;
				itemSelected( droppedCut.clipItem );
			}
			else
				updateTransitions( droppedCut.clipItem, true );
			droppedCut.clipItem->setParentItem( tracks.at( newTrack ) );
			droppedCut.clipItem->setPosition( newPos );
			droppedCut.clip->setPosition( newPos );
			updateTransitions( droppedCut.clipItem, false );
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
			updateTransitions( droppedCut.clipItem, true );
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
			bool empty = topParent->getSampler()->isProjectEmpty();
			scene->addClip( droppedCut.clip, getTrack( droppedCut.clipItem->sceneBoundingRect().topLeft() ) );
			clipThumbRequest( droppedCut.clipItem, true );
			clipThumbRequest( droppedCut.clipItem, false );
			emit updateFrame();
			QTimer::singleShot( 1, this, SLOT(updateLength()) );
			if ( empty ) {
				emit clipAddedToTimeline(droppedCut.clip->getProfile());
			}
			droppedCut.reset();
		}
		else {
			droppedCut.destroy();
		}
	}
	else
		QGraphicsScene::dropEvent( event );
}



void Timeline::clipThumbRequest( ClipViewItem *it, bool start )
{
	Clip *c = it->getClip();
	double pts = c->start();
	bool neg = c->getSpeed() < 0;
	if ( (neg && start) || (!neg && !start) )
		pts = c->start() + (c->length() * qAbs(c->getSpeed())) - c->getProfile().getVideoFrameDuration();

	topParent->clipThumbRequest( ThumbRequest( (void*)it, c->getType(), c->sourcePath(), c->getProfile(), pts ) );
}



void Timeline::thumbResultReady( ThumbRequest result )
{
	if ( !result.thumb.isNull() && result.caller != NULL ) {
		ClipViewItem *it = (ClipViewItem*)result.caller;
		if ( items().contains( it ) ) {
			if ( it->setThumb( result ) && it == selectedItem )
				emit clipSelected( it );
		}
	}
}
