#include <QPainter>
#include <QDebug>

#include "cursorviewitem.h"



CursorViewItem::CursorViewItem() : TypeRectItem( UNDEF )
{
	setRect( 0, 0, 8, TRACKVIEWITEMHEIGHT - 2 );
	
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
