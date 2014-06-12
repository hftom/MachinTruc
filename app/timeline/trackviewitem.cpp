#include "timeline.h"
#include "trackviewitem.h"



TrackViewItem::TrackViewItem() : TypeRectItem( UNDEF )
{
	setRect( 0, 0, 1, TRACKVIEWITEMHEIGHT );

	setPen( QColor(255, 255, 200) );
	setBrush( QColor(255, 255, 200) );
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




