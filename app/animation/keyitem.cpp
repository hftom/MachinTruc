#include "animitem.h"



KeyItem::KeyItem( AnimItem *parent ) : QGraphicsRectItem( parent ),
	anim( parent )
{
	setRect( 0, 0, HALFKEYSIZE * 2, HALFKEYSIZE * 2 );

	normalPen.setColor( QColor("black") );
	selectionPen.setColor( QColor("magenta") );
	normalBrush.setStyle( Qt::SolidPattern );
	normalBrush.setColor( Qt::transparent );
	selectionBrush.setStyle( Qt::SolidPattern );
	selectionBrush.setColor( QColor(255, 0, 255, 64 ) );
	
	setSelected( false );
}



void KeyItem::setSelected( bool b )
{
	selected = b;
	if ( selected ) {
		setPen( selectionPen );
		setBrush( selectionBrush );
	}
	else {
		setPen( normalPen );
		setBrush( normalBrush );
	}
}



void KeyItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
	QGraphicsRectItem::paint( painter, option, widget );
}



void KeyItem::mousePressEvent( QGraphicsSceneMouseEvent *event )
{
	firstMove = true;
	moveStartPosition = scenePos();
	moveStartMouse = event->scenePos();
	anim->itemSelected( this );
}



void KeyItem::mouseMoveEvent( QGraphicsSceneMouseEvent *event )
{
	if ( firstMove && (event->scenePos() - moveStartMouse).manhattanLength() < SNAPWIDTH )
		return;
	firstMove = false;
	anim->itemMove( this, event->scenePos(), moveStartPosition, moveStartMouse );
}



void KeyItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *event )
{
	Q_UNUSED( event );
}



void KeyItem::mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event )
{
	Q_UNUSED( event );
	
	anim->keyDoubleClicked( this );
}
