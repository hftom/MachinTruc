#include <QPainter>
#include <QDebug>
#include <QCursor>

#include "timeline.h"
#include "typeitem.h"
#include "cursorviewitem.h"



CursorViewItem::CursorViewItem()
{
	setData( DATAITEMTYPE, TYPECURSOR );
	setRect( 0, 0, 10, TRACKVIEWITEMHEIGHT - 2 );
	setAcceptHoverEvents(true);
	
	setPen( QColor(0, 255, 0, 100) );
	setBrush( QColor(0, 255, 0, 100) );
	
	trackMarker = QPixmap(":/images/icons/point.png");
	setActiveTrack( 0 );
}



void CursorViewItem::setActiveTrack( int t )
{
	activeTrack = qMax(t, 0);
	markerYPos = rect().height() - 4 - (TRACKVIEWITEMHEIGHT / 2) - (activeTrack * TRACKVIEWITEMHEIGHT);
	update();
}



void CursorViewItem::setHeight( double h )
{
	QRectF r = rect();
	r.setHeight( h );
	setRect( r );
	update();
}



void CursorViewItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
	QGraphicsRectItem::paint( painter, option, widget );
	
	QRectF r = rect();
	painter->drawPixmap(QPointF(r.left() + 1.0, markerYPos), trackMarker);
	painter->setPen( QColor(0,0,0) );
	painter->drawLine( r.topLeft(), r.bottomLeft() );
}



void CursorViewItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->buttons() & Qt::RightButton ) {
		setActiveTrack((rect().height() - event->pos().y() - 5 - RULERDOCKHEIGHT) / TRACKVIEWITEMHEIGHT);
		return;
	}
	startMoveOffset = event->scenePos().x() - mapToScene( rect().topLeft() ).x();
}



void CursorViewItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	if ( event->buttons() & Qt::RightButton ) {
		return;
	}
	isMoving = true;
	Timeline *t = (Timeline*)scene();
	t->playheadMoved( event->scenePos().x() - startMoveOffset );
}



void CursorViewItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	isMoving = false;
	Timeline *t = (Timeline*)scene();
	t->playheadMoved( event->scenePos().x() - startMoveOffset );
}



void CursorViewItem::hoverMoveEvent( QGraphicsSceneHoverEvent *event )
{
	setCursor( QCursor(Qt::SizeHorCursor) );
}
