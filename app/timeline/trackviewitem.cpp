#include "timeline.h"
#include "trackviewitem.h"



TrackViewItem::TrackViewItem() : TypeRectItem( UNDEF )
{
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
	t->trackPressed( event->scenePos() );
}




