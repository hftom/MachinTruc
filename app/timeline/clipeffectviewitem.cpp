#include <math.h>

#include "clipeffectviewitem.h"



ClipEffectViewItem::ClipEffectViewItem() : TypeRectItem( UNDEF )
{
	setAcceptHoverEvents(true);

	normalPen.setJoinStyle( Qt::MiterJoin );
	normalPen.setColor( "lime" );

	QLinearGradient grad( QPointF(0, 0), QPointF(0, 1) );
	grad.setCoordinateMode( QGradient::ObjectBoundingMode );
	grad.setColorAt( 0, QColor(200,255,0,180) );
	grad.setColorAt( 1, QColor(128,178,0,180) );
	normalBrush = QBrush( grad );
	
	setPen( normalPen );
	setBrush( normalBrush );
}



void ClipEffectViewItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	Q_UNUSED( event );
	
	/*firstMove = true;
	if ( event->pos().x() < SNAPWIDTH ) {
		moveResize = 1;
		moveStartLength = length;
		moveStartPosition = position;
	}
	else if ( event->pos().x() > rect().width() - SNAPWIDTH ) {
		moveResize = 2;
		moveStartLength = length;
		moveStartPosition = position;
	}
	else {
		moveResize = 0;
		moveStartPosition = position;
	}		
	moveStartMouse = event->scenePos();
	Timeline* t = (Timeline*)scene();
	t->itemSelected( this );*/
}



void ClipEffectViewItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	Q_UNUSED( event );
	
	/*if ( firstMove && fabs( event->scenePos().x() - moveStartMouse.x() ) < SNAPWIDTH )
		return;
	firstMove = false;
	Timeline* t = (Timeline*)scene();
	if ( moveResize )
		t->clipItemCanResize( this, moveResize, event->scenePos(), moveStartPosition, moveStartLength, moveStartMouse );
	else
		t->clipItemCanMove( this, event->scenePos(), moveStartPosition, moveStartMouse );*/
}



void ClipEffectViewItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	Q_UNUSED( event );
	
	/*if ( firstMove )
		return;
	Timeline* t = (Timeline*)scene();
	if ( moveResize )
		t->clipItemResized( this, moveResize );
	else
		t->clipItemMoved( this, moveStartMouse );*/
}



void ClipEffectViewItem::hoverMoveEvent( QGraphicsSceneHoverEvent *event )
{
	Q_UNUSED( event );
	
	/*if ( event->pos().x() < SNAPWIDTH || event->pos().x() > rect().width() - SNAPWIDTH ) {
		setCursor( Qt::SplitHCursor );
	}
	else {
		setCursor( Qt::PointingHandCursor );
	}*/
}
