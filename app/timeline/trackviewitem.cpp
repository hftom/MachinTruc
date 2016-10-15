#include <QPainter>

#include "timeline.h"
#include "typeitem.h"
#include "trackviewitem.h"



TrackViewItem::TrackViewItem()
{
	setData( DATAITEMTYPE, TYPETRACK );
	setRect( 0, 0, 1, TRACKVIEWITEMHEIGHT );
	
	QLinearGradient grad( QPointF(0, 0), QPointF(0, 1) );
	grad.setCoordinateMode( QGradient::ObjectBoundingMode );
	grad.setColorAt( 0, "#8E8E8E" );
	grad.setColorAt( 1, "#ECECEC" );

	setPen( QColor( "#ECECEC" ) );
	setBrush( QBrush( grad ) );
}



void TrackViewItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
	QGraphicsRectItem::paint( painter, option, widget );
	
	QRectF r = rect();
	painter->setPen( QColor(0,0,0) );
	painter->drawLine( r.topLeft(), r.topRight() );
}



void TrackViewItem::mousePressEvent( QGraphicsSceneMouseEvent * event )
{
	Timeline *t = (Timeline*)scene();
	if (event->modifiers() & Qt::ShiftModifier) {
		t->trackSelectWindow( event->scenePos() );
	}
	else if ( event->button() == Qt::LeftButton ) {
		t->trackPressed( event->scenePos() );
	}
	else if ( event->button() == Qt::RightButton ) {
		t->trackPressedRightBtn( this, event->screenPos() );
	}
}



void TrackViewItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	Timeline *t = (Timeline*)scene();
	if (event->modifiers() & Qt::ShiftModifier) {
		t->trackSelectWindowMove( event->scenePos() );
	}
}



void TrackViewItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	Timeline *t = (Timeline*)scene();
	t->trackSelectWindowRelease(event->modifiers() & Qt::ControlModifier);
}
