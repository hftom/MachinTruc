#include <QDebug>
#include <QMimeData>
#include <QInputDialog>
#include <QMessageBox>

#include "undoclipadd.h"
#include "undoclipremove.h"
#include "undoclipmove.h"
#include "undoclipresize.h"
#include "undoclipspeed.h"
#include "undoclipsplit.h"
#include "undoeffectadd.h"
#include "undoeffectremove.h"
#include "undotrackadd.h"
#include "undoeffectmove.h"
#include "undoeffectresize.h"
#include "undoeffectreorder.h"
#include "undoeffectparam.h"
#include "undotransitionchanged.h"

#include "engine/filtercollection.h"
#include "gui/mimetypes.h"
#include "gui/topwindow.h"
#include "gui/addclipsdialog.h"
#include "timeline/timeline.h"

#define MINTRACKLENGTH 300
#define DEFAULTZOOM 102400.0
#define ZOOMMAX 3276800.0
#define ZOOMMIN 800.0



Timeline::Timeline( TopWindow *parent, QUndoStack *stack ) : QGraphicsScene(),
	zoom( DEFAULTZOOM ),
	currentZoom( DEFAULTZOOM ),
	viewWidth( 0 ),
	effectItem( NULL ),
	scene( NULL ),
	topParent( parent ),
	selectWindowItem( NULL ),
	forceEnsureVisible( false ),
	undoStack(stack)
{
	setBackgroundBrush( QBrush( QColor(20,20,20) ) );
	
	rulerDock = new RulerDock();
	addItem( rulerDock );
	
	cursor = new CursorViewItem();
	addItem( cursor );
	cursor->setZValue( ZCURSOR );
	cursor->setPos( 0, 0 );
	
	ruler = new RulerViewItem();
	addItem( ruler );
	ruler->setZValue( ZRULER );
	ruler->setPos( 0, 0 );
	ruler->setTimeScale( MICROSECOND / zoom );
	
	zoomAnim = new QPropertyAnimation( this, "animZoom" );
	zoomAnim->setDuration( 250 );
	zoomAnim->setEasingCurve( QEasingCurve::InOutSine );
	
	mouseScenePosition = QPointF( -1, -1 );
}



Timeline::~Timeline()
{
}



void Timeline::viewMouseMove( QPointF pos )
{
	int t = qMax( 0, getTrack( pos ) );
	int tc = tracks.count();
	double y;

	if ( pos.y() < RULERDOCKHEIGHT )
		t = tc - 1;

	if ( ruler->isDocked() || t == tc - 1)
		y = 0;
	else
		y = (double)(tc -1 - t) * (TRACKVIEWITEMHEIGHT + 1);

	ruler->setPosition( qMax( 0.0, pos.x() - RULERWIDTH / 2 ), y );

	mouseScenePosition = QPointF( pos.x(), t );
}



void Timeline::undockRuler()
{
	ruler->undock();
}



void Timeline::dockRuler()
{
	ruler->dock();
}



void Timeline::viewMouseLeave()
{
	ruler->setPosition( ruler->pos().x(), 0 );
	
	mouseScenePosition = QPointF( -1, -1 );
}



void Timeline::viewSizeChanged( const QSize &size )
{
	viewWidth = size.width();
	updateAfterEdit( false, true );
}



void Timeline::wheelEvent( QGraphicsSceneWheelEvent *e )
{
	if ( e->modifiers() & Qt::ControlModifier ) {
		zoomInOut( e->delta() > 0 );
		e->accept();
	}
}



void Timeline::zoomInOut( bool in )
{
	double oldzoom = zoom;

	if ( in ) {
		if ( zoom > ZOOMMIN )
			zoom /= 2.0;
	}
	else {
		if ( zoom < ZOOMMAX )
			zoom *= 2;
	}

	if ( oldzoom != zoom ) {
		zoomAnim->stop();
		zoomAnim->setKeyValueAt( 0, getCurrentZoom() );
		zoomAnim->setKeyValueAt( 1, zoom );
		zoomAnim->start();
	}
}



void Timeline::setCurrentZoom( qreal z )
{
	int i, j;
	for ( i = 0; i < tracks.count(); ++i ) {
		TrackViewItem *tv = tracks.at( i );
		QList<QGraphicsItem*> list = tv->childItems();
		for ( j = 0; j < list.count(); ++j ) {
			AbstractViewItem *it = (AbstractViewItem*)list.at( j );
			it->setScale( z );
		}
	}
	ruler->setTimeScale( MICROSECOND / z );
	cursor->setX( (cursor->x() * currentZoom) / z );
	updateLength();
	emit centerOn( cursor );
	currentZoom = z;
	update();
}



qreal Timeline::getCurrentZoom()
{
	return currentZoom;
}



void Timeline::selectAll()
{
	int n = 0, total = 0;

	for ( int j = 0; j < tracks.count(); ++j ) {
		QList<QGraphicsItem*> list = tracks.at( j )->childItems();
		for ( int i = 0; i < list.count(); ++i ) {
			QGraphicsItem *it = list.at( i );
			if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
				++total;
			}
		}
	}
	for ( int j = 0; j < tracks.count(); ++j ) {
		QList<QGraphicsItem*> list = tracks.at( j )->childItems();
		for ( int i = 0; i < list.count(); ++i ) {
			QGraphicsItem *it = list.at( i );
			if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
				ClipViewItem *cv = (ClipViewItem*)it;
				itemSelected((ClipViewItem*)it, n > 0, n < total - 1);
				++n;
			}
		}
	}
}



void Timeline::itemSelected( AbstractViewItem *it, bool extend, bool moreToCome )
{
	if ( effectItem ) {
		removeItem( effectItem );
		delete effectItem;
		effectItem = NULL;
	}

	if (!it || !extend) {
		while (selectedItems.count()) {
			selectedItems.takeFirst()->setSelected( 0 );
		}
	}
	if ( it ) {
		int pos = selectedItems.indexOf(it);
		if (pos == -1) {
			if (!selectedItems.count() || !extend) {
				it->setSelected( 2 );
			}
			else {
				it->setSelected( 1 );
			}
			selectedItems.append(it);
		}
		else {
			it->setSelected( 0 );
			selectedItems.takeAt( pos );
			if ( selectedItems.count() ) {
				selectedItems.first()->setSelected( 2 );
			}
		}
	}
	
	if (!moreToCome) {
		emit clipSelected( selectedItems.count() 
			? selectedItems.first()->data( DATAITEMTYPE ).toInt() == TYPECLIP ? (ClipViewItem*)selectedItems.first() : NULL
			: NULL
		);
	}
}



ClipViewItem* Timeline::getSelectedClip()
{
	ClipViewItem *it = NULL;
	if ( selectedItems.count() ) {
		if ( selectedItems.first()->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			it = (ClipViewItem*)selectedItems.first();
		}
	}
	
	return it;
}



void Timeline::trackPressed( QPointF p )
{
	double pts = p.x() * zoom;
	qint64 i = pts / scene->getProfile().getVideoFrameDuration();
	pts = i;
	emit seekTo( pts * scene->getProfile().getVideoFrameDuration() );
}



void Timeline::trackSelectWindow( QPointF p )
{
	if ( selectWindowItem ) {
		removeItem( selectWindowItem );
		delete selectWindowItem;
		selectWindowItem = NULL;
	}
	
	selectWindowItem = new SelectWindowItem( p );
	addItem( selectWindowItem );
}



void Timeline::trackSelectWindowMove( QPointF p )
{
	if (selectWindowItem) {
		selectWindowItem->setEndPoint( p );
	}
}



void Timeline::trackSelectWindowRelease(bool extend)
{
	if ( !selectWindowItem ) {
		return;
	}
	
	QList<QGraphicsItem *> list = selectWindowItem->collidingItems();
	int n = 0;
	for (int i = 0; i < list.count(); ++i) {
		QGraphicsItem *it = list.at(i);
		if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			++n;
		}
	}
	if (!extend) {
		itemSelected(NULL);
	}
	for (int i = 0; i < list.count(); ++i) {
		QGraphicsItem *it = list.at(i);
		if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
			itemSelected( (ClipViewItem*)it, extend || i > 0, i < n - 1 );
		}
	}
	
	removeItem( selectWindowItem );
	delete selectWindowItem;
	selectWindowItem = NULL;
}



void Timeline::playheadMoved( double p )
{
	double pts = p * zoom;
	double d = scene->getProfile().getVideoFrameDuration();
	qint64 i = pts / d;
	pts = i;
	pts *= d;
	emit seekTo( pts );
	
	i = ( pts + ( d / 2.0 ) ) / d;
	pts = i * d;
	cursor->setX( qBound(0.0, pts / zoom, scene->getDuration() / zoom) );
	
}



void Timeline::setCursorPos( double pts, bool isPlaying )
{
	double d = scene->getProfile().getVideoFrameDuration();
	qint64 i = ( pts + ( d / 2.0 ) ) / d;
	pts = i * d;
	if (!cursor->cursorIsMoving()) {
		cursor->setX( pts / zoom );
	}
	if (isPlaying || forceEnsureVisible) {
		emit ensureVisible( cursor );
		forceEnsureVisible = false;
	}
}




void Timeline::nextEdge()
{
	double pts = cursor->x() * zoom;
	double spts = scene->nextEdge(pts);
	
	if (effectItem) {
		QSharedPointer<Filter> f;
		if ( effectItem->isVideoEffect() )
			f = effectItem->getClip()->videoFilters.at( effectItem->getIndex() );
		else
			f = effectItem->getClip()->audioFilters.at(  effectItem->getIndex() );
		
		double margin = scene->getProfile().getVideoFrameDuration() / 4.0;
		double edge = f->getPosition() + f->getPositionOffset() - margin;
		
		if (edge > pts && edge < spts) {
			spts = edge + margin;
		}
		else {
			edge += f->getLength();
			if (edge > pts && edge < spts) {
				spts = edge + margin;
			}
		}
	}
	
	forceEnsureVisible = true;
	emit seekTo( spts );
}



void Timeline::previousEdge()
{
	double pts = cursor->x() * zoom;
	double spts = scene->previousEdge(pts);
	
	if (effectItem) {
		QSharedPointer<Filter> f;
		if ( effectItem->isVideoEffect() )
			f = effectItem->getClip()->videoFilters.at( effectItem->getIndex() );
		else
			f = effectItem->getClip()->audioFilters.at(  effectItem->getIndex() );
		
		double margin = scene->getProfile().getVideoFrameDuration() / 4.0;
		double edge = f->getPosition() + f->getPositionOffset() + f->getLength() + margin;
		
		if (edge < pts && edge > spts) {
			spts = edge;
		}
		else {
			edge -= f->getLength();
			if (edge < pts && edge > spts) {
				spts = edge;
			}
		}
	}
	
	forceEnsureVisible = true;
	emit seekTo( spts );
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
	UndoTrackAdd *u = new UndoTrackAdd(this, index, true);
	undoStack->push(u);
}



void Timeline::trackAdded( int index )
{
	addTrack( index );
}



void Timeline::addTrack( int index, bool noUndo )
{
	if (noUndo) {
		// we are called from setScene to add a track view for the already existing track model
		// so we call the above with true as last param to not create the track model again 
		commandTrackAddRemove(index, false, true);
	}
	else {
		UndoTrackAdd *u = new UndoTrackAdd(this, index, false);
		undoStack->push(u);
	}
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
	
	ClipViewItem *cv = (ClipViewItem*)selectedItems.first();
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

	Clip *c = cv->getClip();
	double oldSpeed = c->getSpeed();
	double oldLength = c->length();
	double speed = QInputDialog::getDouble( topParent, tr("Clip speed"), tr("Set a negative value to reverse."), oldSpeed, -10.0, 10.0, 2 );
	if ( speed == 0 )
		speed = 0.01;
	if ( speed != oldSpeed ) {
		double newLength = (1.0 / qAbs(speed)) * c->length() / (1.0 / qAbs(oldSpeed));
		int track = getTrack( cv->sceneBoundingRect().topLeft() );
		// set new speed now, since scene->canResize needs it
		// and restore oldSpeed if we can't resize
		// We set a positive speed so that scene->canResize
		// does not change clip->start
		c->setSpeed( qAbs(speed) );
		Transition *tail = scene->getTailTransition(c, track);
		Transition *oldTail = tail ? new Transition(tail) : NULL;
		if ( scene->canResize( c, newLength, track ) ) {
			updateTransitions( cv, true );
			cv->setLength( newLength );
			updateTransitions( cv, false );
			scene->resize( c, newLength, track );
			// set the real speed now
			c->setSpeed( speed );
			clipThumbRequest( cv, true );
			clipThumbRequest( cv, false );
			// force scene update
			scene->update = true;
			UndoClipSpeed *u = new UndoClipSpeed(this, c, track, oldSpeed, speed, oldLength, c->length(), oldTail, scene->getTailTransition(c, track), true);
			undoStack->push(u);
			QTimer::singleShot ( 1, this, SIGNAL(updateFrame()) );
			QTimer::singleShot ( 1, this, SLOT(updateLength()) );
		}
		else {
			QMessageBox::warning( topParent, tr("Warning"), tr("There is not enougth room to resize this clip. Move next clips and try again.") );
			c->setSpeed( oldSpeed );
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
	QSharedPointer<Filter> f;
	if ( effectItem->isVideoEffect() )
		f = effectItem->getClip()->videoFilters.at( effectItem->getIndex() );
	else
		f = effectItem->getClip()->audioFilters.at( effectItem->getIndex() );

	double oldPosOffset = f->getPositionOffset();
	scene->effectMove( effectItem->getClip(), effectItem->getPosition(), effectItem->isVideoEffect(), effectItem->getIndex() );
	
	if (oldPosOffset != f->getPositionOffset()) {
		UndoEffectMove *u = new UndoEffectMove(this, effectItem->getClip(), f->getPosition() + oldPosOffset,
											   f->getPosition() + f->getPositionOffset(), effectItem->isVideoEffect(), effectItem->getIndex(), true);
		undoStack->push(u);
	}

	updateAfterEdit(true, false);
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
	QSharedPointer<Filter> f;
	if ( effectItem->isVideoEffect() )
		f = effectItem->getClip()->videoFilters.at( effectItem->getIndex() );
	else
		f = effectItem->getClip()->audioFilters.at( effectItem->getIndex() );

	double oldLen = f->getLength();
	double oldOffset = f->getPositionOffset();

	if ( way == 2 )
		scene->effectResize( effectItem->getClip(), effectItem->getLength(), effectItem->isVideoEffect(), effectItem->getIndex() );
	else
		scene->effectResizeStart( effectItem->getClip(), effectItem->getPosition(), effectItem->getLength(), effectItem->isVideoEffect(), effectItem->getIndex() );
	
	UndoEffectResize *u = new UndoEffectResize(this, effectItem->getClip(), way != 2, oldOffset, f->getPositionOffset(), f->getPosition(),
											   oldLen, f->getLength(), effectItem->isVideoEffect(), effectItem->getIndex(), true);
	undoStack->push(u);

	updateAfterEdit(true, false);
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
	int newTrack = getTrack( clip->sceneBoundingRect().topLeft() );
	int oldTrack = multiMove ? newTrack : getTrack( clipStartMouse );
	Clip *c = clip->getClip();
	double oldPos = c->position();
	Transition *oldTrans = c->getTransition() ? new Transition(c->getTransition()) : NULL;
	Transition *tail = multiMove ? NULL : scene->getTailTransition(c, oldTrack);
	Transition *oldTail = tail ? new Transition(tail) : NULL;
	if ( multiMove ) {
		scene->moveMulti( clip->getClip(), getTrack( clip->sceneBoundingRect().topLeft() ), clip->getPosition() );
	}
	else {
		scene->move( c, oldTrack, clip->getPosition(), newTrack );
	}
	
	if (oldPos == c->position()) {
		if (oldTrans) delete oldTrans;
		if (oldTail) delete oldTail;
		return;
	}
	
	UndoClipMove *u = new UndoClipMove(this, c, multiMove, oldTrack, newTrack, oldPos, c->position(), oldTrans, c->getTransition(),
									   oldTail, multiMove ? NULL : scene->getTailTransition(c, newTrack), true);
	undoStack->push(u);
	if (oldTrans) delete oldTrans;
	if (oldTail) delete oldTail;
	
	updateAfterEdit(true, true);
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
	int track = getTrack( clip->sceneBoundingRect().topLeft() );
	Transition *oldTrans, *newTrans;
	Clip *c = clip->getClip();
	double oldPos = c->position();
	double oldLen = c->length();
	if ( way == 2 ) {
		Transition *trans = scene->getTailTransition(c, track);
		oldTrans = trans ? new Transition(trans) : NULL;
		scene->resize( c, clip->getLength(), track );
		newTrans = scene->getTailTransition(c, track);
	}
	else {
		Transition *trans = c->getTransition();
		oldTrans = trans ? new Transition(trans) : NULL;
		scene->resizeStart( c, clip->getPosition(), clip->getLength(), track );
		newTrans = c->getTransition();
	}
	
	if (oldLen == c->length()) {
		if (oldTrans) delete oldTrans;
		return;
	}
	
	clipThumbRequest( clip, way != 2 );
	UndoClipResize *u = new UndoClipResize(this, c,  way != 2, track, oldPos, c->position(), oldLen, c->length(), oldTrans, newTrans, true);
	undoStack->push(u);
	if (oldTrans) delete oldTrans;
	
	updateAfterEdit(true, true);
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



ClipViewItem* Timeline::getClipViewItem(Clip *clip, int track)
{
	// search in given track
	if (track >= 0) {
		QList<QGraphicsItem*> list = tracks.at( track )->childItems();
		for ( int i = 0; i < list.count(); ++i ) {
			QGraphicsItem *it = list.at( i );
			if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
				ClipViewItem *cv = (ClipViewItem*)it;
				if ( cv->getClip() == clip ) {
					return cv;
				}
			}
		}
	}
	// search in all tracks
	for ( int j = 0; j < tracks.count(); ++j ) {
		QList<QGraphicsItem*> list = tracks.at( j )->childItems();
		for ( int i = 0; i < list.count(); ++i ) {
			QGraphicsItem *it = list.at( i );
			if ( it->data( DATAITEMTYPE ).toInt() == TYPECLIP ) {
				ClipViewItem *cv = (ClipViewItem*)it;
				if ( cv->getClip() == clip ) {
					return cv;
				}
			}
		}
	}

	//QMessageBox::warning( topParent, tr("Warning"), QString("no clipviewitem found for clip %1 on track %2, position %3")
	//	.arg(clip->sourcePath()).arg(track).arg(clip->position()/MICROSECOND));
	return NULL;
}



void Timeline::updateAfterEdit(bool doFrame, bool doLength)
{
	if (doLength) {
		updateLength();
	}
	if (doFrame) {
		emit updateFrame();
	}
}



void Timeline::slotUpdateAfterEdit()
{
	updateAfterEdit(true, true);
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
	
	QRectF r = rulerDock->rect();
	r.setWidth( maxlen );
	rulerDock->setRect( r );
	setSceneRect( 0, 0, maxlen, tracks.count() * TRACKVIEWITEMHEIGHT );
}



void Timeline::setScene( Scene *s )
{
	int i, j;

	scene = s;
	itemSelected( NULL );

	while ( tracks.count() ) {
		QGraphicsItem *it = tracks.takeFirst();
		removeItem( it );
		delete it;
	}

	for ( i = 0; i < s->tracks.count(); ++i ) {
		addTrack( i, true );
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
	
	setCursorPos( 0, true );
	ruler->setFrameDuration( scene->getProfile().getVideoFrameDuration() );

	updateAfterEdit(false, true);
}



void Timeline::editCopy(ClipBoard *clipboard)
{
	if (!selectedItems.count()) {
		return;
	}
	
	QList< QList<Clip*>* > *list = new QList< QList<Clip*>* >();
	
	for (int i = 0; i < tracks.count(); ++i) {
		list->append(new QList<Clip*>());
	}
	
	for (int i = 0; i < selectedItems.count(); ++i) {
		if (selectedItems.at(i)->data( DATAITEMTYPE ).toInt() == TYPECLIP) {
			ClipViewItem *cv = (ClipViewItem*)selectedItems.at(i);
			Clip *c = cv->getClip();
			int t = getTrack( cv->sceneBoundingRect().topLeft() );
			list->at(t)->append(c);
		}
	}
	
	while (!list->isEmpty() && list->first()->isEmpty()) {
		delete list->takeFirst();
	}
	while (!list->isEmpty() && list->last()->isEmpty()) {
		delete list->takeLast();
	}
	
	clipboard->copyClips(list);
	
	while (!list->isEmpty())
		delete list->takeFirst();
	delete list;
}



void Timeline::editCut(ClipBoard *clipboard)
{
	editCopy(clipboard);

	int n = 0;
	if ( selectedItems.count() ) {
		UndoClipRemove *u = new UndoClipRemove(this);
		for (int i = 0; i < selectedItems.count(); ++i) {
			if (selectedItems.at(i)->data( DATAITEMTYPE ).toInt() == TYPECLIP) {
				++n;
				ClipViewItem *cv = (ClipViewItem*)selectedItems.at(i);
				Clip *c = cv->getClip();
				int t = getTrack( cv->sceneBoundingRect().topLeft() );
				u->append(c, t, scene->getTailTransition(c, t));
			}
		}
		if (n) {
			undoStack->push(u);
		}
		else {
			delete u;
		}
	}
}



void Timeline::editPaste(ClipBoard *clipboard)
{
	QString type = clipboard->getCopyType();

	if (type == "AudioFilter" || type == "VideoFilter") {
		int n = 0;
		if ( selectedItems.count() && !clipboard->getFilter().isNull() ) {
			UndoEffectAdd *u = new UndoEffectAdd(this);
			for (int i = 0; i < selectedItems.count(); ++i) {
				if (selectedItems.at(i)->data( DATAITEMTYPE ).toInt() == TYPECLIP) {
					ClipViewItem *cv = (ClipViewItem*)selectedItems.at(i);
					Clip *c = cv->getClip();
					QSharedPointer<Filter> f = clipboard->getFilter();
					int index = -1;
					if ( (f->getIdentifier().startsWith("GL") && !c->getSource()->getProfile().hasVideo())
						|| (!f->getIdentifier().startsWith("GL") && !c->getSource()->getProfile().hasAudio())
					) {
						continue;
					}
					if (f->getIdentifier() == "GLStabilize") {
						if (c->getSource()->getType() != InputBase::FFMPEG) {
							continue;
						}
						// put GLStabilize at top of effect list
						index = 0;
					}
					++n;
					u->append(c, getTrack(cv->sceneBoundingRect().topLeft()), f, index, type == "VideoFilter");
					f->setPosition( c->position() );
					if ( f->getLength() > c->length() )
						f->setLength( c->length() );
					if ( f->getSnap() == Filter::SNAPEND )
						f->setPositionOffset( c->length() - f->getLength() );
					else if ( f->getSnap() == Filter::SNAPSTART )
						f->setPositionOffset( 0 );
					else
						f->setLength( c->length() );
				}
			}
			if (n) {
				undoStack->push(u);
			}
			else {
				delete u;
			}
		}
	}
	else if (type.startsWith("Track")) {
		QList< QList<Clip*>* > *list = clipboard->getClips( topParent->getAllSources(), scene );
		
		int activeTrack = cursor->getActiveTrack();
		if (mouseScenePosition.y() >= 0 && mouseScenePosition.x() >= 0) {
			activeTrack = mouseScenePosition.y();
		}
		if ((activeTrack + 1 - list->count() < 0)) {
			clipboard->deleteClips(list);
			return;
		}
		
		double oldPos = -1;
		for (int i = 0; i < list->count(); ++i) {
			if (list->at(i)->count()) {
				double pos = list->at(i)->first()->position();
				if ( pos < oldPos || oldPos < 0 ) {
					oldPos = pos;
				}
			}
		}
		
		int trackBase = activeTrack - list->count() + 1;
		double cursorPos = 0;
		if (mouseScenePosition.y() >= 0 && mouseScenePosition.x() >= 0) {
			cursorPos = mouseScenePosition.x() ;
		}
		else {
			cursorPos = cursor->mapRectToScene( cursor->rect() ).left();
		}
		cursorPos = (cursorPos * zoom) + (scene->getProfile().getVideoFrameDuration() / 4.0);
		double offset = cursorPos - oldPos;
		QList<Clip*> added;
		for (int i = 0; i < list->count(); ++i) {
			QList<Clip*> *track = list->at(i);
			for (int j = 0; j < track->count(); ++j) {
				Clip *c = track->at(j);
				double newPos = c->position() + offset;
				if (scene->canMove( c, c->length(), newPos, trackBase + i )) {
					c->setPosition( newPos );
					scene->addClip(c, trackBase + i);
					added.append(c);
				}
				else {
					while (!added.isEmpty()) {
						scene->removeClip(added.takeFirst());
					}
					clipboard->deleteClips(list);
					return;
				}
			}
		}
		
		int n = 0;
		for (int i = 0; i < list->count(); ++i) {
			QList<Clip*> *track = list->at(i);
			for (int j = 0; j < track->count(); ++j) {
				Clip *clip = track->at(j);
				ClipViewItem *cv = new ClipViewItem( clip, zoom );
				cv->setParentItem( tracks.at( trackBase + i ) );

				updateStabilize(clip, NULL, false);

				updateTransitions( cv, false );
				clipThumbRequest( cv, true );
				clipThumbRequest( cv, false );
				
				itemSelected( cv, n > 0, n < added.count() - 1 );
				++n;
			}
		}

		UndoClipAdd *u = new UndoClipAdd(this, true);
		for (int i = 0; i < list->count(); ++i) {
			QList<Clip*> *track = list->at(i);
			for (int j = 0; j < track->count(); ++j) {
				Clip *c = track->at(j);
				u->append( c, trackBase + i, scene->getTailTransition(c, trackBase + i));
			}
		}
		undoStack->push(u);

		updateAfterEdit(true, true);
	}
}



void Timeline::addSelectionToTimeline()
{
	bool empty = topParent->getSampler()->isProjectEmpty();
	QList<Source*> list = topParent->getSelectedSources();
	if (!list.count()) {
		return;
	}
	
	AddClipsDialog dlg(NULL);
	dlg.exec();
	if ( dlg.result() != QDialog::Accepted ) {
		return;
	}
	AddClipsSettings settings = dlg.getSettings();
	
	int activeTrack = cursor->getActiveTrack();
	double clipPos = cursor->mapRectToScene( cursor->rect() ).left();
	clipPos = (clipPos * zoom) + (scene->getProfile().getVideoFrameDuration() / 4.0);
	
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	double pw = scene->getProfile().getVideoWidth();
	double ph = scene->getProfile().getVideoHeight();
	
	QList<Clip*> added;
	
	for (int j = 0; j < list.count(); ++j) {
		int animType = rand() % 2; 
		Source *source = list.at(j);
		double len = (source->getType() == InputBase::IMAGE ? (double)settings.imageDuration * MICROSECOND : source->getProfile().getStreamDuration());
		Clip *c = scene->createClip( source, clipPos, source->getProfile().getStreamStartTime(), len );
		if (scene->canMove( c, c->length(), clipPos, activeTrack )) {
			c->setPosition( clipPos );
			clipPos += c->length() - (MICROSECOND * settings.transitionDuration);
			qDebug() << clipPos;
			scene->addClip(c, activeTrack);
			if (settings.panAndZoom) {
				QSharedPointer<Filter> f = QSharedPointer<Filter>( new GLPanZoom() );
				f->setPosition( c->position() );
				f->setLength( c->length() );
				c->videoFilters.append( f.staticCast<GLFilter>() );
			}
			added.append(c);
		}
		else {
			while (!added.isEmpty()) {
				Clip *dc = added.takeFirst();
				scene->removeClip(dc);
				delete dc;
			}
			return;
		}
	}
	
	int n = 0;
	for (int j = 0; j < added.count(); ++j) {
		Clip *clip = added.at(j);
		ClipViewItem *cv = new ClipViewItem( clip, zoom );
		cv->setParentItem( tracks.at( activeTrack ) );
		
		updateStabilize(clip, NULL, false);
		
		updateTransitions( cv, false );
		clipThumbRequest( cv, true );
		clipThumbRequest( cv, false );
		
		itemSelected( cv, n > 0, n < added.count() - 1 );
		++n;
	}
	
	UndoClipAdd *u = new UndoClipAdd(this, true);
	for (int j = 0; j < added.count(); ++j) {
		Clip *c = added.at(j);
		u->append( c, activeTrack, scene->getTailTransition(c, activeTrack));
	}
	undoStack->push(u);
	
	updateAfterEdit(true, true);
	if ( empty ) {
		emit clipAddedToTimeline(added.first()->getProfile());
	}
}



void Timeline::splitCurrentClip()
{
	ClipViewItem *cv = getSelectedClip();
	if ( cv ) {
		Clip *current_clip = cv->getClip();
		double cursor_pts = cursor->mapRectToScene( cursor->rect() ).left() * zoom ;
		int t = getTrack( cv->sceneBoundingRect().topLeft() );
		if (!scene->canSplitClip(current_clip, t, cursor_pts)) {
			return;
		}
		Clip *dup = scene->duplicateClip(current_clip);
		Transition *startTrans = current_clip->getTransition() ? new Transition(current_clip->getTransition()) : NULL;
		Transition *trans = scene->getTailTransition(current_clip, t);
		Transition *tail = trans ? new Transition(trans) : NULL;
		updateTransitions(cv, true);
		itemSelected(NULL);
		removeItem(cv);
		delete cv;
		updateStabilize( current_clip, NULL, true);
		scene->removeClip(current_clip);
		scene->addClip(dup, t);
		if (startTrans) {
			dup->setTransition(new Transition(startTrans));
		}
		if (tail) {
			Clip *next = scene->getTailClip(dup, t);
			next->setTransition(new Transition(tail));
		}
		Clip *c = scene->sceneSplitClip( dup, t, cursor_pts );
		UndoClipSplit *u = new UndoClipSplit(this, current_clip, dup, c, t, startTrans, tail, true);
		undoStack->push(u);
		if (tail) {
			delete tail;
		}
		if (startTrans) {
			delete startTrans;
		}
		ClipViewItem *cv1 = new ClipViewItem( dup, zoom );
		cv1->setParentItem( tracks.at( t ) );
		updateTransitions(cv1, false);
		ClipViewItem *cv2 = new ClipViewItem( c, zoom );
		cv2->setParentItem( tracks.at( t ) );
		updateTransitions(cv2, false);
		itemSelected( cv2 );
		clipThumbRequest( cv1, true );
		clipThumbRequest( cv1, false );
		clipThumbRequest( cv2, true );
		clipThumbRequest( cv2, false );
		
		updateAfterEdit(true, false);
	}
}



void Timeline::addFilter( ClipViewItem *clip, QString fx, int index )
{
	int i;
	bool isVideo = true;
	Clip *c = clip->getClip();

	if ( fx == "GLStabilize" && c->getSource()->getType() != InputBase::FFMPEG ) {
		return;
	}
	
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	QSharedPointer<Filter> f = fc->createVideoFilter(fx);
	if ( !f.isNull() ) {
		if ( !c->getSource()->getProfile().hasVideo() ) {
			return;
		}
	}
	else {
		if ( !c->getSource()->getProfile().hasAudio() ) {
			return;
		}
		f = fc->createAudioFilter(fx);
		isVideo = false;
	}
	
	if ( f.isNull() ) {
		return;
	}
	
	f->setPosition( c->position() );
	if ( f->getLength() > c->length() ) {
		f->setLength( c->length() );
	}
	if ( f->getSnap() == Filter::SNAPEND ) {
		f->setPositionOffset( c->length() - f->getLength() );
	}
	else if ( f->getSnap() == Filter::SNAPSTART ) {
		f->setPositionOffset( 0 );
	}
	else {
		f->setLength( c->length() );
	}
	
	UndoEffectAdd *u = new UndoEffectAdd(this);
	u->append( c, getTrack(clip->sceneBoundingRect().topLeft()), f, index, isVideo );
	undoStack->push(u);
}



void Timeline::filterDeleted( Clip *c, QSharedPointer<Filter> f )
{
	bool isVideo = true;
	ClipViewItem *cv = getClipViewItem(c, -1);
	int index = -1;
	
	for ( int i = 0; i < c->videoFilters.count(); ++i ) {
		if ( c->videoFilters.at( i ).data() == f.data() ) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		for ( int i = 0; i < c->audioFilters.count(); ++i ) {
			if ( c->audioFilters.at(i).data() == f.data() ) {
				index = i;
				isVideo = false;
				break;
			}
		}
	}
	
	if (index == -1)
		return;

	UndoEffectRemove *u = new UndoEffectRemove(this);
	u->append(c, getTrack(cv->sceneBoundingRect().topLeft()), f, index, isVideo);
	undoStack->push(u);
}



void Timeline::filterReordered( Clip *c, bool video, int index, int newIndex )
{
	ClipViewItem *cv = getSelectedClip();
	if ( cv ) {
		if (cv->getClip() != c) {
			cv = getClipViewItem(c, -1);
		}
		if (cv) {
			UndoEffectReorder *u = new UndoEffectReorder(this, c, getTrack( cv->sceneBoundingRect().topLeft() ), index, newIndex, video);
			undoStack->push(u);
		}
	}
}



void Timeline::dragEnterEvent( QGraphicsSceneDragDropEvent *event )
{
	const QMimeData *mimeData = event->mimeData();
	QString t;
	if ( mimeData->formats().contains( MIMETYPESOURCE ) )
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
			int t = getTrack( droppedCut.clipItem->sceneBoundingRect().topLeft() );
			scene->addClip( droppedCut.clip, t );
			UndoClipAdd *u = new UndoClipAdd(this, true);
			u->append(droppedCut.clip, t, scene->getTailTransition(droppedCut.clip, t));
			undoStack->push(u);
			clipThumbRequest( droppedCut.clipItem, true );
			clipThumbRequest( droppedCut.clipItem, false );
			emit updateFrame();
			updateAfterEdit(false, true);
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
	if ( (neg && start) || (!neg && !start) ) {
		pts = c->start() + (c->length() * qAbs(c->getSpeed())) - c->getProfile().getVideoFrameDuration();
	}

	topParent->clipThumbRequest( ThumbRequest( (void*)it, c->getType(), c->sourcePath(), c->getProfile(), pts ) );
}



void Timeline::thumbResultReady( ThumbRequest result )
{
	if ( !result.thumb.isNull() && result.caller != NULL ) {
		ClipViewItem *it = (ClipViewItem*)result.caller;
		if ( items().contains( it ) ) {
			ClipViewItem *selected = getSelectedClip();
			if ( it->setThumb( result ) && it == selected )
				emit clipSelected( selected );
		}
	}
}



void Timeline::updateStabilize(Clip *clip, Filter *f, bool stop)
{
	if (stop) {
		for ( int i = 0; i < clip->videoFilters.count(); ++i ) {
			QSharedPointer<Filter> filter = clip->videoFilters.at( i );
			if ( filter->getIdentifier() == "GLStabilize" && (!f || filter.data() == f) ) {
				filter.staticCast<GLStabilize>()->filterRemoved();
			}
		}
	}
	else {
		for ( int i = 0; i < clip->videoFilters.count(); ++i ) {
			QSharedPointer<Filter> filter = clip->videoFilters.at( i );
			if ( filter->getIdentifier() == "GLStabilize" && (!f || filter.data() == f) ) {
				filter.staticCast<GLStabilize>()->setSource(clip->getSource());
			}
		}
	}
}



void Timeline::commandAddClip(QList<Clip*> clips, QList<int> ltracks, QList<Transition*> tails)
{
	QList<ClipViewItem*> cvs;
	for (int i = 0; i < clips.count(); ++i) {
		int track = ltracks.at(i);
		Clip *clip = clips.at(i);
		
		ClipViewItem *cv = new ClipViewItem( clip, zoom );
		cvs.append(cv);
		cv->setParentItem( tracks.at( track ) );
		scene->addClip( clip, track );

		updateStabilize(clip, NULL, false);

		updateTransitions( cv, false );
		clipThumbRequest( cv, true );
		clipThumbRequest( cv, false );
	}
	// reset transitions after all clips have been added.
	for (int i = 0; i < clips.count(); ++i) {
		int track = ltracks.at(i);
		Clip *clip = clips.at(i);
		Transition *tail = tails.at(i);

		Clip *next = scene->getTailClip(clip, track);
		if (next) {
			next->setTransition(tail ? new Transition(tail) : NULL);
		}
	}
	for (int i = 0; i < cvs.count(); ++i) {
		itemSelected( cvs.at(i), i > 0, i < cvs.count() - 1 );
	}
	
	updateAfterEdit(true, true);
}



void Timeline::commandRemoveClip(QList<Clip*> clips, QList<int> ltracks)
{
	itemSelected( NULL );
	for (int i = 0; i < clips.count(); ++i) {
		int track = ltracks.at(i);
		Clip *clip = clips.at(i);
		
		ClipViewItem *cv = getClipViewItem(clip, track);
		if (cv) {
			updateStabilize(clip, NULL, true);
			if ( scene->removeClip( cv->getClip() ) ) {
				updateTransitions( cv, true );
				removeItem( cv );
				delete cv;
			}
		}
	}
	
	QTimer::singleShot(500, this, SLOT(slotUpdateAfterEdit()));
}



void Timeline::commandMoveClip(Clip *clip, bool multi, int oldTrack, int newTrack, double pos, Transition *trans, Transition *tail)
{
	ClipViewItem *cv = getClipViewItem(clip, oldTrack);
	if ( cv ) {
		updateTransitions( cv, true );
		if (multi) {
			double start = clip->position();
			double delta = pos - start;
			scene->moveMulti( clip, newTrack, pos );
			QList<QGraphicsItem*> list = tracks.at( newTrack )->childItems();
			for ( int i = 0; i < list.count(); ++i ) {
				QGraphicsItem *it = list.at( i );
				if ( it->data( DATAITEMTYPE ).toInt() >= TYPECLIP ) {
					AbstractViewItem *av = (AbstractViewItem*)it;
					if ( av->getPosition() < start )
						continue;
					av->moveDelta( delta );
				}
			}
		}
		else {
			scene->move( clip, oldTrack, pos, newTrack );
			cv->setParentItem( tracks.at( newTrack ) );
			cv->setCuts( clip->position(), clip->length(), zoom );
		}
		clip->setTransition(trans ? new Transition(trans) : NULL);
		if (!multi) {
			Clip *next = scene->getTailClip(clip, newTrack);
			if (next) {
				next->setTransition(tail ? new Transition(tail) : NULL);
			}
		}
		updateTransitions( cv, false );
		
		updateAfterEdit(true, true);
	}
}



void Timeline::commandResizeClip(Clip *clip, bool resizeStart, int track, double position, double length, Transition *trans)
{
	ClipViewItem *cv = getClipViewItem(clip, track);
	if ( cv ) {
		if (resizeStart) {
			scene->resizeStart( clip, position, length, track );
			clip->setTransition(trans ? new Transition(trans) : NULL);
			updateTransitions( cv, true );
			cv->setGeometry( position, length );
			updateTransitions( cv, false );
		}
		else {
			scene->resize( clip, length, track );
			Clip *next = scene->getTailClip(clip, track);
			if (next) {
				next->setTransition(trans ? new Transition(trans) : NULL);
			}
			updateTransitions( cv, true );
			cv->setLength( length );
			updateTransitions( cv, false );
		}
		clipThumbRequest( cv, resizeStart );
		
		updateAfterEdit(true, true);
	}
}



void Timeline::commandClipSpeed(Clip *c, int track, double speed, double length, Transition *tail)
{
	ClipViewItem *cv = getClipViewItem(c, track);
	c->setSpeed( qAbs(speed) );
	updateTransitions( cv, true );
	cv->setLength( length );
	updateTransitions( cv, false );
	scene->resize( c, length, track );
	Clip *next = scene->getTailClip(c, track);
	if (next) {
		next->setTransition(tail ? new Transition(tail) : NULL);
	}
	// set the real speed now
	c->setSpeed( speed );
	clipThumbRequest( cv, true );
	clipThumbRequest( cv, false );
	// force scene update
	scene->update = true;

	updateAfterEdit(true, true);
}



void Timeline::commandSplitClip(Clip *c, Clip *c1, Clip *c2, int track, Transition *trans, Transition *tail, bool redo)
{
	itemSelected( NULL );
	if (redo) {
		ClipViewItem *cv = getClipViewItem(c, track);
		updateTransitions( cv, true );
		removeItem(cv);
		delete cv;
		updateStabilize(c, NULL, true);
		scene->removeClip(c);
		updateStabilize(c1, NULL, false);
		scene->addClip(c1, track);
		updateStabilize(c2, NULL, false);
		scene->addClip(c2, track);
		if (trans) {
			c1->setTransition(new Transition(trans));
		}
		if (tail) {
			Clip *next = scene->getTailClip(c2, track);
			next->setTransition(new Transition(tail));
		}
		ClipViewItem *cv1 = new ClipViewItem( c1, zoom );
		cv1->setParentItem( tracks.at( track ) );
		updateTransitions( cv1, false );
		ClipViewItem *cv2 = new ClipViewItem( c2, zoom );
		cv2->setParentItem( tracks.at( track ) );
		updateTransitions( cv2, false );
		itemSelected(cv2);
		clipThumbRequest( cv1, true );
		clipThumbRequest( cv1, false );
		clipThumbRequest( cv2, true );
		clipThumbRequest( cv2, false );
	}
	else {
		ClipViewItem *cv1 = getClipViewItem(c1, track);
		updateTransitions( cv1, true );
		removeItem(cv1);
		delete cv1;
		ClipViewItem *cv2 = getClipViewItem(c2, track);
		updateTransitions( cv2, true );
		removeItem(cv2);
		delete cv2;
		updateStabilize(c1, NULL, true);
		scene->removeClip(c1);
		updateStabilize(c2, NULL, true);
		scene->removeClip(c2);
		updateStabilize(c, NULL, false);
		scene->addClip(c, track);
		if (trans) {
			c->setTransition(new Transition(trans));
		}
		if (tail) {
			Clip *next = scene->getTailClip(c, track);
			next->setTransition(new Transition(tail));
		}
		ClipViewItem *cv = new ClipViewItem( c, zoom );
		cv->setParentItem( tracks.at( track ) );
		updateTransitions( cv, false );
		itemSelected(cv);
		clipThumbRequest( cv, true );
		clipThumbRequest( cv, false );
	}
	updateAfterEdit(true, false);
}



void Timeline::commandEffectAddRemove(QList<Clip*> clips, QList<int> ltracks, QList< QSharedPointer<Filter> > filters, bool isVideo, QList<int> indexes, bool remove)
{
	if (remove) {
		for (int i = 0; i < clips.count(); ++i) {
			Clip *c = clips.at(i);
			QSharedPointer<Filter> f = filters.at(i);
			
			updateStabilize(c, f.data(), true);
			if (isVideo) {
				c->videoFilters.remove( f.staticCast<GLFilter>() );
			}
			else
				c->audioFilters.remove( f.staticCast<AudioFilter>() );
		}
	}
	else {
		for (int i = 0; i < clips.count(); ++i) {
			Clip *c = clips.at(i);
			QSharedPointer<Filter> f = filters.at(i);
			int index = indexes.at(i);
		
			if (isVideo) {
				if ( index == -1 )
					c->videoFilters.append( f.staticCast<GLFilter>() );
				else
					c->videoFilters.insert( index, f.staticCast<GLFilter>() );
			}
			else {
				if ( index == -1 )
					c->audioFilters.append( f.staticCast<AudioFilter>() );
				else
					c->audioFilters.insert( index, f.staticCast<AudioFilter>() );
			}
			updateStabilize(c, f.data(), false);
		}
	}

	for (int i = 0; i < clips.count(); ++i) {
		itemSelected(getClipViewItem(clips.at(i), ltracks.at(i)), i > 0, i < clips.count() - 1);
	}
	updateAfterEdit(true, false);
}



void Timeline::commandTrackAddRemove(int index, bool remove, bool noparent)
{
	if ( !noparent ) // special for setScene() that doesn't set a undo command and has already the track
		topParent->timelineTrackAddRemove(index, remove);

	if (remove) {
		QGraphicsItem *it = tracks.takeAt( index );
		removeItem( it );
		delete it;
		for ( int i = 0; i < tracks.count(); ++i )
			tracks.at( tracks.count() - 1 - i )->setPos( 0, ((TRACKVIEWITEMHEIGHT + 1) * i) + RULERDOCKHEIGHT );
	}
	else {
		TrackViewItem *tv = new TrackViewItem();
		tracks.insert( index, tv );
		addItem( tv );
		int i;
		for ( i = 0; i < tracks.count(); ++i )
			tracks.at( tracks.count() - 1 - i )->setPos( 0, ((TRACKVIEWITEMHEIGHT + 1) * i) + RULERDOCKHEIGHT );
	}
	
	cursor->setHeight( (tracks.count() * (TRACKVIEWITEMHEIGHT + 1)) + RULERDOCKHEIGHT );
	cursor->setActiveTrack(0);
	updateAfterEdit(false, true);
}



void Timeline::commandEffectMove(Clip *c, double newPos, bool isVideo, int index)
{
	scene->effectMove( c, newPos, isVideo, index );
	if (effectItem) {
		effectItem->setPosition(newPos);
	}

	updateAfterEdit(true, false);
}



void Timeline::commandEffectResize(Clip *c, bool resizeStart, double offset, double position, double length, bool video, int effectIndex)
{
	if (resizeStart) {
		scene->effectResizeStart( c, position + offset, length, video, effectIndex );
	}
	else {
		scene->effectResize( c, length, video, effectIndex );
	}

	if (effectItem) {
		effectItem->setGeometry(position + offset, length);
	}

	updateAfterEdit(true, false);
}



void Timeline::commandEffectReorder(Clip *c, int track, int oldIndex, int newIndex, bool isVideo)
{
	ClipViewItem *cv = getClipViewItem(c, track);
	if (isVideo) {
		c->videoFilters.move(oldIndex, newIndex);
	}
	else {
		c->audioFilters.move(oldIndex, newIndex);
	}
	
	itemSelected(cv);
	updateAfterEdit(true, false);
}



void Timeline::paramUndoCommand(QSharedPointer<Filter> f, Parameter *p, QVariant oldValue, QVariant newValue)
{
	UndoEffectParam *u = new UndoEffectParam(this, f, p, oldValue, newValue);
	undoStack->push(u);
}



void Timeline::commandEffectParam(QSharedPointer<Filter> filter, Parameter *param, QVariant value)
{
	param->value = value;
	itemSelected(getSelectedClip());
	if ( param->type == Parameter::PSHADEREDIT ) {
		GLCustom *f = (GLCustom*) filter.data();
		f->setCustomParams( value.toString() );
	}
	
	updateAfterEdit(true, false);
}



void Timeline::transitionChanged(Clip *clip, QString filterName, bool isVideo)
{
	QSharedPointer<Filter> f;
	if (isVideo) {
		f = clip->getTransition()->getVideoFilter();
	}
	else {
		f = clip->getTransition()->getAudioFilter();
	}
	UndoTransitionChanged *u = new UndoTransitionChanged(this, clip, f, filterName, isVideo);
	undoStack->push(u);
}



void Timeline::commandTransitionChanged(Clip *clip, QSharedPointer<Filter> oldFilter, QString newFilter, bool isVideo, bool undo)
{
	FilterCollection *fc = FilterCollection::getGlobalInstance();
	
	if ( undo ) {
		if (isVideo) {
			clip->getTransition()->setVideoFilter( oldFilter.staticCast<GLFilter>() );
		}
		else {
			clip->getTransition()->setAudioFilter( oldFilter.staticCast<AudioFilter>() );
		}
	}
	else {
		if (isVideo) {
			for ( int i = 0; i < fc->videoTransitions.count(); ++i ) {
				if ( fc->videoTransitions[ i ].name == newFilter ) {
					QSharedPointer<Filter> f = fc->videoTransitions[ i ].create();
					clip->getTransition()->setVideoFilter( f.staticCast<GLFilter>() );
					break;
				}
			}
		}
		else {
			for ( int i = 0; i < fc->audioTransitions.count(); ++i ) {
				if ( fc->audioTransitions[ i ].name == newFilter ) {
					QSharedPointer<Filter> f = fc->audioTransitions[ i ].create();
					clip->getTransition()->setAudioFilter( f.staticCast<AudioFilter>() );
					break;
				}
			}
		}
	}
	
	ClipViewItem *selected = getSelectedClip();
	if ( selected ) {
		emit clipSelected( selected );
	}
	updateAfterEdit(true, false);
}
