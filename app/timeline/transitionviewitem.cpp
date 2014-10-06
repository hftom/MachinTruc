#include <math.h>

#include <QCursor>

#include "transitionviewitem.h"



TransitionViewItem::TransitionViewItem( QGraphicsItem *parent, double pos, double len, double scale )
{
	setData( DATAITEMTYPE, TYPETRANSITION );	
	setParentItem( parent );
	setZValue( ZTRANSITION );

	//setAcceptHoverEvents(true);
	
	setCuts( pos, len, scale );
	
	normalPen.setJoinStyle( Qt::MiterJoin );
	normalPen.setColor( "lime" );

	QLinearGradient grad( QPointF(0, 0), QPointF(0, 1) );
	grad.setCoordinateMode( QGradient::ObjectBoundingMode );
	grad.setColorAt( 0, QColor(200,255,0,100) );
	grad.setColorAt( 1, QColor(128,178,0,100) );
	normalBrush = QBrush( grad );
	
	setPen( normalPen );
	setBrush( normalBrush );
}



void TransitionViewItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
	QGraphicsRectItem::paint( painter, option, widget );
}
