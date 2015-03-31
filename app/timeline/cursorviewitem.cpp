#include <QPainter>
#include <QDebug>
#include <QCursor>

#include "timeline.h"
#include "typeitem.h"
#include "cursorviewitem.h"



CursorViewItem::CursorViewItem()
{
	setData( DATAITEMTYPE, TYPECURSOR );
	setRect( 0, 0, 8, TRACKVIEWITEMHEIGHT - 2 );
	setAcceptHoverEvents(true);
	
	setPen( QColor(0, 255, 0, 128) );
	setBrush( QColor(0, 255, 0, 128) );
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
	painter->setPen( QColor(0,0,0) );
	painter->drawLine( r.topLeft(), r.bottomLeft() );
}



void CursorViewItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	startMoveOffset = event->scenePos().x() - mapToScene( rect().topLeft() ).x();
}



void CursorViewItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	Timeline *t = (Timeline*)scene();
	t->playheadMoved( event->scenePos().x() - startMoveOffset );
}



void CursorViewItem::hoverMoveEvent( QGraphicsSceneHoverEvent *event )
{
	setCursor( QCursor(Qt::SizeHorCursor) );
}
