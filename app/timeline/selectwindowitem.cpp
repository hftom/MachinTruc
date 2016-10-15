#include <QPainter>
#include <QDebug>

#include "typeitem.h"
#include "selectwindowitem.h"



SelectWindowItem::SelectWindowItem( QPointF start )
{
	setData( DATAITEMTYPE, TYPETRACK );
	setZValue( ZSELECTWINDOW );
	startX = start.x();
	startY = start.y();
	setRect( startX, startY, 0, 0 );

	setPen( QColor(132,0,66,128) );
	setBrush( QBrush( QColor(132,0,66,128) ) );
}



void SelectWindowItem::setEndPoint( QPointF end )
{
	double x = startX;
	double y = startY;
	double x2 = end.x();
	double y2 = end.y();
	if (x > x2) {
		x2 = x;
		x = end.x();
	}
	if (y > y2) {
		y2 = y;
		y = end.y();
	}

	setRect( x, y, x2 - x, y2 - y );
}
